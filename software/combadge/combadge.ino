#include <WiFi.h>
#include "driver/adc.h"
#include "config.h"
#include "src/i2scfg.h"
#include "src/crap.h"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"
#include "src/max98357.h"

#ifdef MIC_SPH0645
#include "src/sph0645.h"
#else
#include "src/inmp441.h"
#endif

WiFiClient conn;

MAX98357 spk;

#ifdef MIC_SPH0645
SPH0645 mic;
#else
INMP441 mic;
#endif

volatile bool touched = false;

TaskHandle_t streamFromMicHandle;

void IRAM_ATTR touchISR() {
#ifdef SOC_ESP32
    if (!touched) {
        touched = true;
    }
#else
    touched = touchInterruptGetLastStatus(TOUCH_PIN);
#endif
}

void setup() {
    setCpuFrequencyMhz(80);
    btStop();
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    establishConnection();

    I2SCfg i2scfg = {
        .sampleRate = SAMPLE_RATE,
        .bitsPerSample = BITS_PER_SAMPLE,
    };

    MicPinCfg micPins = {
        .bclk = MIC_BCLK,
        .ws = MIC_WS,
        .data = MIC_DATA,
    };
    if (!mic.begin(MIC_PORT, i2scfg, micPins)) {
        Serial.println("Failed initializing microphone");
        while (true)
            blinkCycle(100);
    }

    MAX98357PinCfg spkPins = {
        .bclk = SPK_BCLK,
        .ws = SPK_WS,
        .data = SPK_DATA,
        .enable = SPK_EN,
    };
    if (!spk.begin(SPK_PORT, i2scfg, spkPins)) {
        Serial.println("Failed initializing speaker");
        while (true)
            blinkCycle(100);
    }

    playSound(TNGChirp1, TNGChirp1SizeBytes);

    xTaskCreate(streamFromMic, "StreamFromMic", 10240, NULL, 0, &streamFromMicHandle);

    touchAttachInterrupt(TOUCH_PIN, touchISR, TOUCH_THRESHOLD);
}

void loop() {
    static bool receiving = false;
    static uint32_t lastPacketMillis = 0;
    static AudioPacket ad;

    bool gotBadPacket = false;
    size_t headerBytesRecvd = 0;

    if (conn.available() >= sizeof(PacketHeader))
        headerBytesRecvd = conn.read((uint8_t*) &ad.header, sizeof(PacketHeader));

    if (headerBytesRecvd == sizeof(PacketHeader)) {
        gotBadPacket = false;
        switch (ad.header.type) {
            case AUDIO_START:
                spk.wake();
                Serial.println("Starting playback");
                playSound(HailBeep, HailBeepSizeBytes);
                receiving = true;
                break;

            case AUDIO_STOP:
                spk.sleep();
                Serial.println("Stopping playback");
                receiving = false;
                break;

            case AUDIO_DATA:
                {
                    size_t totalBytesRead = 0, bytesWritten = 0;
                    ad.header.size = min(ad.header.size, BUF_LEN_BYTES);

                    while (totalBytesRead < ad.header.size) {
                        size_t bytesRead = conn.read((uint8_t*) ad.data, ((size_t) ad.header.size) - totalBytesRead);
                        totalBytesRead += bytesRead;

                        spk.write((uint8_t*) ad.data, bytesRead, &bytesWritten);
                        if (bytesRead != bytesWritten)
                            Serial.printf("Wrote only %d of %d bytes to speaker\r\n", bytesWritten, bytesRead);
                    }
                }
                break;

            default:
                gotBadPacket = true;
        }

        if (gotBadPacket) {
            //Serial.printf("Bad packet type 0x%x with size %d\r\n", ad.header.type, ad.header.size);
        } else {
            lastPacketMillis = millis();
            return;
        }
    }

    if (receiving && millis() - lastPacketMillis > TX_DROP_TIMEOUT_MS) {
        Serial.println("Transmission dropped");
        receiving = false;
        spk.sleep();
        while (conn.available()) conn.read(); // Clear inbound buffer
    }

    if (!conn.connected()) {
        Serial.println("Connection lost");
        receiving = false;
        spk.sleep();
        establishConnection();
    }
}

void streamFromMic(void*) {
    static AudioPacket audio = {};
    audio.header.type = AUDIO_DATA;

    while (true) {
        while (!touched || !conn.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); }
        playSound(TNGChirp1, TNGChirp1SizeBytes);
        waitTillTouchReleased();

        Serial.println("Starting transmission");
        PacketHeader startMsg = {AUDIO_START, 0};
        conn.write((uint8_t*) &startMsg, sizeof(startMsg));

        while (conn.connected() && !touched) {
            size_t samplesRead = mic.read(audio.data, BUF_LEN_SAMPLES);
            if (samplesRead) {
                audio.header.type = AUDIO_DATA;
                audio.header.size = samplesRead * BYTES_PER_SAMPLE;
                conn.write((uint8_t*) &audio, sizeof(audio.header) + audio.header.size);
            }
        }

        Serial.println("Ending transmission");
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

void establishConnection() {
    if (!WiFi.isConnected()) {
        WiFi.setSleep(true);
        WiFi.setAutoReconnect(true);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        while (!WiFi.isConnected()) {
            blinkCycle(150);
            Serial.print(".");
        }
        Serial.println("Connected to WiFi!");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.macAddress());
    }

    conn.stop();
    while (!conn.connected()) {
        blinkCycle(200);
        conn.connect(BRIDGE, LISTEN_PORT);
    }
    conn.setNoDelay(true);
    Serial.print("Connected to "); Serial.println(BRIDGE);
}

void blinkCycle(int dur_ms) {
    digitalWrite(LED, HIGH);
    vTaskDelay(dur_ms / portTICK_PERIOD_MS);
    digitalWrite(LED, LOW);
    vTaskDelay(dur_ms / portTICK_PERIOD_MS);
}
