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

    playSound(HailBeep, HailBeepSizeBytes);

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
    if (shouldTransmit) {
        size_t incomingBytes;
        if (mic.read((uint8_t*) outgoingBuf, BUF_LEN*BYTES_PER_SAMPLE, &incomingBytes)) {
            udp.writeTo((uint8_t*) outgoingBuf, incomingBytes, BUDDY_IP, UDP_PORT);
        }
        shouldTransmit = false;
    }
}

void onPacket(AsyncUDPPacket packet) {
    size_t bytesRead = packet.read((uint8_t*) incomingBuf, BYTES_PER_SAMPLE*BUF_LEN);

    size_t bytesWritten;
    spk.wake();
    if (!spk.write((uint8_t*) incomingBuf, bytesRead*BYTES_PER_SAMPLE, &bytesWritten)) {
        Serial.println(bytesWritten / BYTES_PER_SAMPLE);
    }
    spk.sleep();
}

void playSound(const sample_t* sound, const size_t soundSizeBytes) {
    int i = 0;
    size_t bytesWritten;
    size_t soundSizeSamples = soundSizeBytes / size_t(BYTES_PER_SAMPLE);

    spk.wake();
    for (i=0; i<soundSizeSamples/BUF_LEN; i++) {
        spk.write((uint8_t*) &(sound[i*BUF_LEN]), BUF_LEN*BYTES_PER_SAMPLE, &bytesWritten);
    }
    spk.write((uint8_t*) &(sound[i*BUF_LEN + soundSizeSamples%BUF_LEN]), (soundSizeSamples%BUF_LEN)*BYTES_PER_SAMPLE, &bytesWritten);
    spk.sleep();
}
