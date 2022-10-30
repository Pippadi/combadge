#include <WiFi.h>
#include <AsyncUDP.h>
#include "config.h"

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"
#define UDP_PORT 1592

#define BUF_LEN 256

AsyncUDP udp;

uint8_t incomingBuf[BUF_LEN];

void onPacket(AsyncUDPPacket packet) {
    size_t size = packet.read(incomingBuf, BUF_LEN);
    Serial.println(size);
    Serial.println(incomingBuf[0]);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to WiFi!");
	Serial.println(WiFi.localIP());

    udp.onPacket(onPacket);
    Serial.println(udp.listen(UDP_PORT));
}

void loop() {}
