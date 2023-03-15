#include <WiFi.h>
#include <AsyncUDP.h>
#include "config.h"
#include "src/i2scfg.h"
#include "src/max98357.h"
#include "src/inmp441.h"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"

WiFiServer server(LISTEN_PORT);
WiFiClient client;
WiFiClient incomingConn;

MAX98357 spk;
INMP441 mic;

sample_t outgoingBuf[BUF_LEN];
sample_t incomingBuf[BUF_LEN];
volatile bool shouldTransmit = false;

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t streamToSpeakerHandle;

void IRAM_ATTR flagOffTransmit() {
    portENTER_CRITICAL_ISR(&timerMux);
    shouldTransmit = true;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    Serial.begin(115200);

    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (!WiFi.isConnected()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");
    Serial.println(WiFi.localIP());

    I2SCfg i2scfg = {
        .sampleRate = SAMPLE_RATE,
        .bitsPerSample = BITS_PER_SAMPLE,
        .bufLen = BUF_LEN,
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

    timer = timerBegin(MIC_TIMER, 80, true);
    timerAttachInterrupt(timer, &flagOffTransmit, true);
    timerAlarmWrite(timer, BUF_FULL_INTERVAL, true);
    timerAlarmEnable(timer);

    xTaskCreate(streamToSpeaker, "StreamToSpeaker", 10240, NULL, 0, &streamToSpeakerHandle);
}

void loop() {
    // mic.read needs a 32-bit buffer
    static int32_t holdingBuf[BUF_LEN];
    if (shouldTransmit) {
        size_t incomingBytes;
        if (mic.read(holdingBuf, BUF_LEN, &incomingBytes)) {
            size_t incomingSamples = incomingBytes/sizeof(int32_t);
            for (int i = 0; i < incomingSamples; i++) {
                // mic.read already removes the unused lower 32-BITS_PER_SAMPLE bits
                outgoingBuf[i] = sample_t(holdingBuf[i]);
            }
            if (client.connected()) {
                client.write((uint8_t*) outgoingBuf, BUF_LEN*BYTES_PER_SAMPLE);
            }
        }
        shouldTransmit = false;
    }
}

void streamToSpeaker(void*) {
    server.begin(LISTEN_PORT, 1);
    while (true) {
        while (!incomingConn) {
            incomingConn = server.available();
        }
        Serial.println(incomingConn.remoteIP());
        spk.wake();
        while (incomingConn.connected()) {
            if (incomingConn.available()) {
                size_t bytesRead = incomingConn.read((uint8_t*) incomingBuf, BYTES_PER_SAMPLE*BUF_LEN);
                size_t bytesWritten;
                if (!spk.write((char*) incomingBuf, bytesRead*BYTES_PER_SAMPLE, &bytesWritten)) {
                    Serial.println(bytesWritten / BYTES_PER_SAMPLE);
                }
            }
        }
        spk.sleep();
    }
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    size_t bytesWritten;
    spk.wake();
    spk.write((char*) sound, soundSizeBytes, &bytesWritten);
    spk.sleep();
}
