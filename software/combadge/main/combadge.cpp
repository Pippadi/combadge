#include <stdio.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "driver/touch_sens.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/sys.h"
#include "lwip/err.h"
#include "config.h"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "i2scfg.hpp"
#include "crap.hpp"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"
#include "max98357.hpp"

#define TAG "combadge"

#ifdef MIC_SPH0645
#include "sph0645.hpp"
#else
#include "inmp441.hpp"
#endif

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define millis() (esp_timer_get_time() / 1000)

int conn;

MAX98357 spk;

#ifdef MIC_SPH0645
SPH0645 mic;
#else
INMP441 mic;
#endif

volatile bool touched = false;

TaskHandle_t streamToSpkHandle;
TaskHandle_t streamFromMicHandle;

EventGroupHandle_t wifiEventGroup;
#define WIFI_CONNECTED_BIT BIT0

void initWifi();
void waitTillTouchReleased();
void playSound(const sample_t* sound, const size_t soundSizeBytes);
void streamToSpk(void*);
void streamFromMic(void*);
void wifiEventHandle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void setupTCPSocket();

/*
void IRAM_ATTR touchISR() {
#ifdef SOC_ESP32
    if (!touched) {
        touched = true;
    }
#else
    touched = touchInterruptGetLastStatus(TOUCH_PIN);
#endif
}
*/

extern "C" void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    initWifi();

    I2SCfg i2sCfg = {
        .sampleRate = SAMPLE_RATE,
        .bitsPerSample = BITS_PER_SAMPLE,
    };

    MicPinCfg micPins = {
        .bclk = MIC_BCLK,
        .ws = MIC_WS,
        .data = MIC_DATA,
    };
    if (!mic.begin(MIC_PORT, i2sCfg, micPins)) {
        ESP_LOGE(TAG, "Failed initializing microphone");
        while (true);
    }

    MAX98357PinCfg spkPins = {
        .bclk = SPK_BCLK,
        .ws = SPK_WS,
        .data = SPK_DATA,
        .enable = SPK_EN,
    };
    if (!spk.begin(SPK_PORT, i2sCfg, spkPins)) {
        ESP_LOGE(TAG, "Failed initializing speaker");
        while (true);
    }

    playSound(TNGChirp1, TNGChirp1SizeBytes);

    //xTaskCreatePinnedToCore(streamToSpk, "StreamToSpk", 10240, NULL, 0, &streamToSpkHandle, 0);
    xTaskCreatePinnedToCore(streamFromMic, "StreamFromMic", 10240, NULL, 0, &streamFromMicHandle, 1);

    //touchAttachInterrupt(TOUCH_PIN, touchISR, TOUCH_THRESHOLD);
}

void streamToSpk(void*) {
    static bool receiving = false;
    static uint32_t lastPacketMillis = 0;
    static AudioPacket ad;

    while (true) {
        bool gotBadPacket = false;
        size_t headerBytesRecvd = 0;

        headerBytesRecvd = recv(conn, (uint8_t*) &ad.header, sizeof(PacketHeader), MSG_WAITALL);

        if (headerBytesRecvd == sizeof(PacketHeader)) {
            gotBadPacket = false;
            switch (ad.header.type) {
            case AUDIO_START:
                spk.wake();
                ESP_LOGI(TAG, "Starting playback");
                playSound(HailBeep, HailBeepSizeBytes);
                receiving = true;
                break;

            case AUDIO_STOP:
                spk.sleep();
                ESP_LOGI(TAG, "Stopping playback");
                receiving = false;
                break;

            case AUDIO_DATA: {
                size_t totalBytesRead = 0, bytesWritten = 0;
                ad.header.size = min(ad.header.size, BUF_LEN_BYTES);

                while (totalBytesRead < ad.header.size) {
                    size_t bytesRead = recv(conn, (uint8_t*) ad.data, ad.header.size - totalBytesRead, MSG_WAITALL);
                    totalBytesRead += bytesRead;

                    spk.write((uint8_t*) ad.data, bytesRead, &bytesWritten);
                    if (bytesRead != bytesWritten)
                        ESP_LOGE(TAG, "Wrote only %u of %u bytes to speaker", bytesWritten, bytesRead);
                }
            }
            break;

            default:
                gotBadPacket = true;
            }

            if (gotBadPacket) {
                ESP_LOGE(TAG, "Bad packet type 0x%lx with size %lu", ad.header.type, ad.header.size);
            } else {
                lastPacketMillis = millis();
                return;
            }
        }

        if (receiving && millis() - lastPacketMillis > TX_DROP_TIMEOUT_MS) {
            ESP_LOGI(TAG, "Transmission dropped");
            receiving = false;
            spk.sleep();
        }
    }
}

void streamFromMic(void*) {
    static AudioPacket audio = {};
    audio.header.type = AUDIO_DATA;

    while (true) {
        while (!touched) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        playSound(TNGChirp1, TNGChirp1SizeBytes);
        waitTillTouchReleased();

        ESP_LOGI(TAG, "Starting transmission");
        PacketHeader startMsg = {AUDIO_START, 0};
        send(conn, (uint8_t*) &startMsg, sizeof(startMsg), 0);

        while (!touched) {
            size_t samplesRead = mic.read(audio.data, BUF_LEN_SAMPLES);
            if (samplesRead) {
                audio.header.size = samplesRead * BYTES_PER_SAMPLE;
                send(conn, (uint8_t*) &audio, sizeof(audio.header) + audio.header.size, 0);
            }
        }

        ESP_LOGI(TAG, "Ending transmission");
        PacketHeader stopMsg = {AUDIO_STOP, 0};
        send(conn, (uint8_t*) &stopMsg, sizeof(stopMsg), 0);
        playSound(TNGChirp2, TNGChirp2SizeBytes);
        waitTillTouchReleased();
    }
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    size_t bytesWritten;
    bool spkAsleep = spk.asleep();
    if (spkAsleep)
        spk.wake();
    spk.write((uint8_t*) sound, soundSizeBytes, &bytesWritten);
    if (spkAsleep)
        spk.sleep();
}

void waitTillTouchReleased() {
    while (touched) {
#ifdef SOC_ESP32
        touched = false;
#endif
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void initWifi() {
    wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifiEventHandle,
                    NULL,
                    &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &wifiEventHandle,
                    NULL,
                    &instance_got_ip));

    wifi_config_t wifiConfig = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold = { .authmode = WIFI_AUTH_WPA2_PSK },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifiEventHandle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
        setupTCPSocket();
    }
}

void setupTCPSocket() {
    ESP_LOGI(TAG, "Setting up TCP socket");
    close(conn);

    struct sockaddr_in destAddr;
    inet_pton(AF_INET, BRIDGE, &destAddr.sin_addr.s_addr);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(LISTEN_PORT);

    conn = socket(AF_INET, SOCK_STREAM, 0);
    int err = connect(conn, (struct sockaddr*) &destAddr, sizeof(destAddr));
    if (err != 0) {
        ESP_LOGE(TAG, "Failed to connect to %s", BRIDGE);
        return;
    }

    ESP_LOGI(TAG, "Connected to %s", BRIDGE);
}
