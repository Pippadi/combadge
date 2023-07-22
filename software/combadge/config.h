#include "driver/i2s.h"

#define MIC_WS 25
#define MIC_DATA 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0

#define SPK_WS 13
#define SPK_DATA 27
#define SPK_BCLK 12
#define SPK_EN 26
#define SPK_PORT I2S_NUM_1

typedef int16_t sample_t;
#define BYTES_PER_SAMPLE sizeof(sample_t)
#define BITS_PER_SAMPLE BYTES_PER_SAMPLE * 8

#define SAMPLE_RATE 44100

#define BUF_LEN_SAMPLES 512
#define BUF_LEN_BYTES BUF_LEN_SAMPLES * BYTES_PER_SAMPLE

#define BUF_FULL_INTERVAL_ms int(float(BUF_LEN_SAMPLES) * 1000.0 / float(SAMPLE_RATE))

#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"

#define BRIDGE "192.168.142.40"
#define LISTEN_PORT 1701

#define TOUCH_PIN 14
#define TOUCH_THRESHOLD 30
