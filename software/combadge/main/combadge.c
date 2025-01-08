#include <stdio.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "lwip/sys.h"
#include "lwip/err.h"
#include "config.h"
#include "src/i2scfg.h"
#include "src/crap.h"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"
#include "src/max98357.h"

#define TAG "combadge"

#ifdef MIC_SPH0645
#include "src/sph0645.h"
#else
#include "src/inmp441.h"
#endif

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

WiFiClient conn;

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

void IRAM_ATTR touchISR() {
#ifdef SOC_ESP32
    if (!touched) {
        touched = true;
    }
#else
    touched = touchInterruptGetLastStatus(TOUCH_PIN);
#endif
}

void app_main() {
    setCpuFrequencyMhz(80);
    btStop();

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    establishConnection();

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
        ("Failed initializing microphone");
        while (true)
            blinkCycle(100);
    }

    MAX98357PinCfg spkPins = {
        .bclk = SPK_BCLK,
        .ws = SPK_WS,
        .data = SPK_DATA,
        .enable = SPK_EN,
    };
    if (!spk.begin(SPK_PORT, i2sCfg, spkPins)) {
        ESP_LOGE(TAG, "Failed initializing speaker");
        while (true)
            blinkCycle(100);
    }

    playSound(TNGChirp1, TNGChirp1SizeBytes);

    //xTaskCreatePinnedToCore(streamToSpk, "StreamToSpk", 10240, NULL, 0, &streamToSpkHandle, 0);
    xTaskCreatePinnedToCore(streamFromMic, "StreamFromMic", 10240, NULL, 0, &streamFromMicHandle, 1);

    touchAttachInterrupt(TOUCH_PIN, touchISR, TOUCH_THRESHOLD);

    loop();
}

void streamToSpk(void*) {
    static bool receiving = false;
    static uint32_t lastPacketMillis = 0;
    static AudioPacket ad;

    while (true) {
        bool gotBadPacket = false;
        size_t headerBytesRecvd = 0;

        if (conn.available() >= sizeof(PacketHeader))
            headerBytesRecvd = conn.read((uint8_t*) &ad.header, sizeof(PacketHeader));

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
                    size_t bytesRead = conn.read((uint8_t*) ad.data, ((size_t) ad.header.size) - totalBytesRead);
                    totalBytesRead += bytesRead;

                    spk.write((uint8_t*) ad.data, bytesRead, &bytesWritten);
                    if (bytesRead != bytesWritten)
                        ESP_LOGE(TAG, "Wrote only %d of %d bytes to speaker", bytesWritten, bytesRead);
                }
            }
            break;

            default:
                gotBadPacket = true;
            }

            if (gotBadPacket) {
                ESP_LOGE(TAG, "Bad packet type 0x%x with size %d", ad.header.type, ad.header.size);
            } else {
                lastPacketMillis = millis();
                return;
            }
        }

        if (receiving && millis() - lastPacketMillis > TX_DROP_TIMEOUT_MS) {
            ESP_LOGI(TAG, "Transmission dropped");
            receiving = false;
            spk.sleep();
            while (conn.available()) conn.read(); // Clear inbound buffer
        }

        if (!conn.connected()) {
            ESP_LOGI(TAG, "Connection lost");
            receiving = false;
            spk.sleep();
            establishConnection();
        }
    }
}

void streamFromMic(void*) {
    static AudioPacket audio = {};
    audio.header.type = AUDIO_DATA;

    while (true) {
        while (!touched || !conn.connected()) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        playSound(TNGChirp1, TNGChirp1SizeBytes);
        waitTillTouchReleased();

        ESP_LOGI(TAG, "Starting transmission");
        PacketHeader startMsg = {AUDIO_START, 0};
        conn.write((uint8_t*) &startMsg, sizeof(startMsg));

        while (conn.connected() && !touched) {
            size_t samplesRead = mic.read(audio.data, BUF_LEN_SAMPLES);
            if (samplesRead) {
                audio.header.size = samplesRead * BYTES_PER_SAMPLE;
                conn.write((uint8_t*) &audio, sizeof(audio.header) + audio.header.size);
            }
        }

        ESP_LOGI(TAG, "Ending transmission");
        PacketHeader stopMsg = {AUDIO_STOP, 0};
        conn.write((uint8_t*) &stopMsg, sizeof(stopMsg));
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

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
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
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }

    conn.stop();
    while (!conn.connected()) {
        blinkCycle(200);
        conn.connect(BRIDGE, LISTEN_PORT);
    }
    conn.setNoDelay(true);
    ESP_LOGI(TAG, "Connected to %s", BRIDGE);
}

void blinkCycle(int dur_ms) {
    digitalWrite(LED, HIGH);
    vTaskDelay(dur_ms / portTICK_PERIOD_MS);
    digitalWrite(LED, LOW);
    vTaskDelay(dur_ms / portTICK_PERIOD_MS);
}
