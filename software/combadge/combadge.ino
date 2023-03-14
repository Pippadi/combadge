#include <WiFi.h>
#include <AsyncUDP.h>
#include "config.h"
#include "src/i2scfg.h"
#include "src/max98357.h"
#include "src/inmp441.h"
#include "sounds/HailBeep.h"
#include "sounds/TNGChirp1.h"
#include "sounds/TNGChirp2.h"

AsyncUDP udp;
MAX98357 spk;
INMP441 mic;

sample_t outgoingBuf[BUF_LEN];
sample_t incomingBuf[BUF_LEN];
volatile bool shouldTransmit = false;

hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

uint32_t lastPacketMillis = 0;

void IRAM_ATTR flagOffTransmit() {
	portENTER_CRITICAL_ISR(&timerMux);
    shouldTransmit = true;
	portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
    Serial.begin(115200);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
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

    udp.onPacket(onPacket);
    if (!udp.listen(UDP_PORT)) {
        Serial.println("Failed listening on UDP");
    };

    /*
	timer = timerBegin(MIC_TIMER, 80, true);
	timerAttachInterrupt(timer, &flagOffTransmit, true);
	timerAlarmWrite(timer, BUF_FULL_INTERVAL, true);
    timerAlarmEnable(timer);
    */
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
            udp.writeTo((uint8_t*) outgoingBuf, incomingSamples*BYTES_PER_SAMPLE, BUDDY_IP, UDP_PORT);
        }
        shouldTransmit = false;
    }

    if (millis() - lastPacketMillis < 100 && spk.asleep()) {
        spk.wake();
    }
    else if (millis() - lastPacketMillis > 100 && !spk.asleep()) {
        spk.sleep();
    }
}

void onPacket(AsyncUDPPacket packet) {
    size_t bytesRead = packet.read((uint8_t*) incomingBuf, BYTES_PER_SAMPLE*BUF_LEN);

    size_t bytesWritten;
    if (!spk.write((char*) incomingBuf, bytesRead*BYTES_PER_SAMPLE, &bytesWritten)) {
        Serial.println(bytesWritten / BYTES_PER_SAMPLE);
    }
    lastPacketMillis = millis();
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    int i = 0;
    size_t bytesWritten;
    size_t soundSizeSamples = soundSizeBytes / size_t(BYTES_PER_SAMPLE);

    spk.wake();
    for (i=0; i<soundSizeSamples/BUF_LEN; i++) {
        Serial.println(spk.write((char*) &(sound[i*BUF_LEN]), BUF_LEN*BYTES_PER_SAMPLE, &bytesWritten));
    }
    spk.write((char*) &(sound[i*BUF_LEN]), (soundSizeSamples%BUF_LEN)*BYTES_PER_SAMPLE, &bytesWritten);
    spk.sleep();
}
