#include "driver/i2s.h"

/* Makefile automatically defines one of these symbols
   when defining SOC="esp32" or SOC="esp32s3" */
//#define SOC_ESP32
//#define SOC_ESP32_S3

#define LED 1

//#define MIC_INMP441
#define MIC_SPH0645

#define MIC_WS GPIO_NUM_17
#define MIC_DATA GPIO_NUM_7
#define MIC_BCLK GPIO_NUM_4

#define SPK_WS GPIO_NUM_9
#define SPK_DATA GPIO_NUM_11
#define SPK_BCLK GPIO_NUM_10
#define SPK_EN GPIO_NUM_18

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

// Milliseconds to wait after last recieved packet to assume that
// a transmission has "dropped" (stopped without AUDIO_STOP)
#define TX_DROP_TIMEOUT_MS 5000
