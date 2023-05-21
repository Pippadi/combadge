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

volatile bool shouldTransmit = false;
volatile bool tapped = false;

TaskHandle_t streamFromMicHandle;

sample_t incomingBuf[BUF_LEN];

void IRAM_ATTR registerTap() {
    tapped = true;
}

void setup() {
    setCpuFrequencyMhz(80);
    btStop();
    adc_power_off();
    Serial.begin(115200);

    WiFi.setSleep(true);
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

    server.begin(LISTEN_PORT, 1);
}

void loop() {
    size_t bytesRecvd, bytesWritten;

    while (!incomingConn) {
        incomingConn = server.available();
    }

    Serial.println(incomingConn.remoteIP());
    spk.wake();
    playSound(HailBeep, HailBeepSizeBytes);

    while (incomingConn.connected()) {
        if (incomingConn.available()) {
            bytesRecvd = incomingConn.read((uint8_t*) incomingBuf, BYTES_PER_SAMPLE*BUF_LEN);
            if (!spk.write((char*) incomingBuf, bytesRecvd, &bytesWritten)) {
                Serial.println(bytesWritten / BYTES_PER_SAMPLE);
            }      
        }
    }

    spk.sleep();
}

void streamFromMic(void*) {
    static sample_t outgoingBuf[BUF_LEN];

    while (true) {
        while (!tapped) { vTaskDelay(10 / portTICK_PERIOD_MS); }

        playSound(TNGChirp1, TNGChirp1SizeBytes);

        vTaskDelay(250 / portTICK_PERIOD_MS);
        tapped = false;

        client.connect(BUDDY_IP, LISTEN_PORT);
        while (!client.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); };
        while (client.connected() && !tapped) {
            // Dividing interval by two so that the buffer doesn't fill up before we're ready to send it
            vTaskDelay(BUF_FULL_INTERVAL_ms / 2 / portTICK_PERIOD_MS);
            // Using outgoingBuf directly because sample_t is int16_t already
            size_t samplesRead = mic.read(outgoingBuf, BUF_LEN);
            if (samplesRead) {
                client.write((uint8_t*) outgoingBuf, samplesRead*BYTES_PER_SAMPLE);
            }
        }
        client.stop();
        vTaskDelay(250 / portTICK_PERIOD_MS);
        tapped = false;
        playSound(TNGChirp2, TNGChirp2SizeBytes);
    }
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    size_t bytesWritten;
    bool spkAsleep = spk.asleep();
    if (spkAsleep) { spk.wake(); }
    spk.write((char*) sound, soundSizeBytes, &bytesWritten);
    if (spkAsleep) { spk.sleep(); }
}
