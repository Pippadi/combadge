#include "driver/i2s.h"

/* Makefile automatically defines one of these symbols
   when defining SOC="esp32" or SOC="esp32s3" */
//#define SOC_ESP32
//#define SOC_ESP32_S3

#define LED 1

//#define MIC_INMP441
#define MIC_SPH0645

#define MIC_WS 17
#define MIC_DATA 7
#define MIC_BCLK 4
#define MIC_PORT I2S_NUM_0

#define SPK_WS 9
#define SPK_DATA 11
#define SPK_BCLK 10
#define SPK_EN 18
#define SPK_PORT I2S_NUM_1

typedef int16_t sample_t;
#define BYTES_PER_SAMPLE sizeof(sample_t)
#define BITS_PER_SAMPLE BYTES_PER_SAMPLE * 8

#define SAMPLE_RATE 44100

#define BUF_LEN_SAMPLES 256
#define BUF_LEN_BYTES BUF_LEN_SAMPLES * BYTES_PER_SAMPLE

#define BUF_FULL_INTERVAL_ms int(float(BUF_LEN_SAMPLES) * 1000.0 / float(SAMPLE_RATE))

#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"

#define BRIDGE "192.168.8.100"
#define LISTEN_PORT 1701

#define TOUCH_PIN 8

#ifdef SOC_ESP32
#define TOUCH_THRESHOLD 30
#else
#define TOUCH_THRESHOLD 40000
#endif
