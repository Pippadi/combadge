#include <WiFi.h>
#include <AsyncUDP.h>
#include "driver/i2s.h"

#define MIC_WS 25
#define MIC_SD 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0

#define SAMPLE_RATE 44100
#define BUF_LEN 256
#define BUF_FULL_INTERVAL BUF_LEN * int(1000000.0 * (1.0 / float(44100)))

#define BUDDY_IP IPAddress(192, 168, 1, 40)
#define UDP_PORT 1592

const char* SSID = "YourSSID";
const char* Password = "YourPassword";

AsyncUDP udp;

int16_t outgoingBuf[BUF_LEN];
volatile bool shouldTransmit = false;

hw_timer_t * timer = NULL;
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

    i2s_config_t conf = {};
    conf.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX);
    conf.sample_rate = SAMPLE_RATE;
    conf.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    conf.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    conf.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
    conf.dma_buf_count = 8;
    conf.dma_buf_len = BUF_LEN;
    conf.use_apll = false;
    conf.tx_desc_auto_clear = false;
    conf.fixed_mclk = 0;

    i2s_pin_config_t pins = {};
    pins.bck_io_num = MIC_BCLK;
    pins.ws_io_num = MIC_WS;
    pins.data_in_num = MIC_SD;
    pins.data_out_num = -1;

    esp_err_t err = i2s_driver_install(MIC_PORT, &conf, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("Failed initializing driver");
        while (true);
    }

    err = i2s_set_pin(MIC_PORT, &pins);
    if (err != ESP_OK) {
        Serial.println("Failed setting pins");
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
