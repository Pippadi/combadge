#include <WiFi.h>
#include <AsyncUDP.h>
#include "driver/i2s.h"
#include "config.h"

const char* SSID = "YourSSID";
const char* Password = "YourPassword";

AsyncUDP udp;

int16_t outgoingBuf[BUF_LEN];
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
    delay(1000);

	WiFi.begin(SSID, Password);
    WiFi.setSleep(false);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to WiFi!");
	Serial.println(WiFi.localIP());
    uint8_t ff = 255;
    udp.writeTo(&ff, 1, BUDDY_IP, UDP_PORT);

    if (!setupMicI2s()) {
        Serial.println("Failed initializing driver");
        while (true);
    }

	timer = timerBegin(0, 80, true);
	timerAttachInterrupt(timer, &flagOffTransmit, true);
	timerAlarmWrite(timer, BUF_FULL_INTERVAL, true);
    timerAlarmEnable(timer);
}

void loop() {
    if (shouldTransmit) {
        size_t incomingBytes = 0;
        esp_err_t err = i2s_read(MIC_PORT, outgoingBuf, BUF_LEN * 2, &incomingBytes, 1000);
        if (err == ESP_OK) {
            Serial.println(outgoingBuf[1]);
            udp.writeTo((uint8_t*) outgoingBuf, incomingBytes, BUDDY_IP, UDP_PORT);
        }
        shouldTransmit = false;
    }
}
