#include <WiFi.h>
#include <AsyncUDP.h>
#include "config.h"
#include "src/i2scfg.h"
#include "src/max98357.h"
#include "src/inmp441.h"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"

#define AUDIO_START 0x01
#define AUDIO_STOP 0x02
#define AUDIO_DATA 0x03

WiFiClient conn;

MAX98357 spk;
INMP441 mic;

volatile bool tapped = false;

TaskHandle_t streamFromMicHandle;

void IRAM_ATTR registerTap() {
    tapped = true;
}

void setup() {
    setCpuFrequencyMhz(80);
    btStop();
    adc_power_off();
    Serial.begin(115200);

    establishConnection();

    I2SCfg i2scfg = {
        .sampleRate = SAMPLE_RATE,
        .bitsPerSample = BITS_PER_SAMPLE,
    };

    INMP441PinCfg micPins = {
        .bclk = MIC_BCLK,
        .ws = MIC_WS,
        .data = MIC_DATA,
    };
    if (!mic.begin(MIC_PORT, i2scfg, micPins)) {
        Serial.println("Failed initializing microphone");
        while (true);
    }

    MAX98357PinCfg spkPins = {
    .bclk = SPK_BCLK,
    .ws = SPK_WS,
    .data = SPK_DATA,
    .enable = SPK_EN,
    };
    if (!spk.begin(SPK_PORT, i2scfg, spkPins)) {
        Serial.println("Failed initializing speaker");
        while (true);
    }

    playSound(TNGChirp1, TNGChirp1SizeBytes);

    xTaskCreate(streamFromMic, "StreamFromMic", 10240, NULL, 0, &streamFromMicHandle);

    touchAttachInterrupt(TOUCH_PIN, registerTap, TOUCH_THRESHOLD);
}

void loop() {
    size_t bytesRecvd, bytesWritten;
    static sample_t incomingBuf[BUF_LEN+1];

    if (conn.available()) {
        bytesRecvd = conn.read((uint8_t*) incomingBuf, BYTES_PER_SAMPLE*(BUF_LEN + 1));
        if (bytesRecvd > 0) {
            switch (incomingBuf[0]) { // Packet type
                case AUDIO_START:
                    spk.wake();
                    playSound(HailBeep, HailBeepSizeBytes);
                    break;
                case AUDIO_STOP:
                    spk.sleep();
                    break;
                case AUDIO_DATA:
                    if (!spk.asleep())
                        spk.write((char*) &incomingBuf[1], bytesRecvd - sizeof(sample_t), &bytesWritten);
                    break;
            }
        }
    }
    else if (!conn.connected()) {
        establishConnection();
    }
}

void streamFromMic(void*) {
    static sample_t outgoingBuf[BUF_LEN + 1];

    while (true) {
        while (!tapped || !conn.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); }
        playSound(TNGChirp1, TNGChirp1SizeBytes);
        waitTillTouchReleased();
        conn.write(AUDIO_START);

        while (conn.connected() && !tapped) {
            // Dividing interval by two so that the buffer doesn't fill up before we're ready to send it
            vTaskDelay(BUF_FULL_INTERVAL_ms / 2 / portTICK_PERIOD_MS);
            // Using outgoingBuf directly because sample_t is int16_t already
            size_t samplesRead = mic.read(&outgoingBuf[1], BUF_LEN);
            outgoingBuf[0] = AUDIO_DATA;
            if (samplesRead) {
                conn.write((uint8_t*) outgoingBuf, (samplesRead+1)*BYTES_PER_SAMPLE);
            }
        }
        conn.write(AUDIO_STOP);
        playSound(TNGChirp2, TNGChirp2SizeBytes);
        waitTillTouchReleased();
    }
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    size_t bytesWritten;
    bool spkAsleep = spk.asleep();
    if (spkAsleep)
        spk.wake();
    spk.write((char*) sound, soundSizeBytes, &bytesWritten);
    if (spkAsleep)
        spk.sleep();
}

void waitTillTouchReleased() {
    while (tapped) {
        tapped = false;
        vTaskDelay(100 / portTICK_PERIOD_MS); // Give interrupt 100ms to fire
    }
}

void establishConnection() {
    WiFi.setSleep(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (!WiFi.isConnected()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");
    Serial.println(WiFi.localIP());

    conn.stop();
    conn.connect(BUDDY_IP, LISTEN_PORT);
    while (!conn.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); };
    Serial.print("Connected to "); Serial.println(BUDDY_IP);
}
