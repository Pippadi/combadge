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

typedef struct {
    uint32_t  type; // No padding = good
    uint32_t size;
} PacketHeader;

typedef struct {
    PacketHeader header;
    sample_t data[BUF_LEN_SAMPLES];
} AudioData;

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
    PacketHeader header;
    AudioData ad;

    if (conn.available()) {
        bytesRecvd = conn.read((uint8_t*) &header, sizeof(PacketHeader));
        if (bytesRecvd > 0) {
            switch (header.type) { // Packet type
                case AUDIO_START:
                    spk.wake();
                    Serial.println("Starting playback");
                    playSound(HailBeep, HailBeepSizeBytes);
                    break;
                case AUDIO_STOP:
                    spk.sleep();
                    Serial.println("Stopping playback");
                    break;
                case AUDIO_DATA:
                    Serial.println("Got samples to play back");
                    bytesRecvd = conn.read((uint8_t*) &(ad.data), sizeof(ad.data));
                    if (!spk.asleep())
                        spk.write((char*) &(ad.data), bytesRecvd, &bytesWritten);
                    break;
            }
        }
    }
    else if (!conn.connected()) {
        spk.sleep();
        establishConnection();
    }
}

void streamFromMic(void*) {
    static AudioData audio = {};
    audio.header.type = AUDIO_DATA;

    while (true) {
        while (!tapped || !conn.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); }
        playSound(TNGChirp1, TNGChirp1SizeBytes);
        waitTillTouchReleased();

        PacketHeader startMsg = {AUDIO_START, 0};
        conn.write((uint8_t*) &startMsg, sizeof(startMsg));

        while (conn.connected() && !tapped) {
            // Dividing interval by two so that the buffer doesn't fill up before we're ready to send it
            vTaskDelay(BUF_FULL_INTERVAL_ms / 2 / portTICK_PERIOD_MS);
            // Using outgoingBuf directly because sample_t is int16_t already
            size_t samplesRead = mic.read(audio.data, BUF_LEN_SAMPLES);
            if (samplesRead) {
                audio.header.type = AUDIO_DATA;
                audio.header.size = samplesRead * BYTES_PER_SAMPLE;
                conn.write((uint8_t*) &audio, sizeof(audio.header) + audio.header.size);
            }
        }

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
    conn.connect(BRIDGE, LISTEN_PORT);
    while (!conn.connected()) { vTaskDelay(10 / portTICK_PERIOD_MS); };
    Serial.print("Connected to "); Serial.println(BRIDGE);
}
