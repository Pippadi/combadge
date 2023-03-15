#include "driver/i2s.h"

#define MIC_WS 25
#define MIC_DATA 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0
#define MIC_TIMER 0

#define SPK_WS 13
#define SPK_DATA 27
#define SPK_BCLK 12
#define SPK_EN 26
#define SPK_PORT I2S_NUM_1

typedef int16_t sample_t;
#define BYTES_PER_SAMPLE sizeof(sample_t)
#define BITS_PER_SAMPLE BYTES_PER_SAMPLE * 8

#define SAMPLE_RATE 44100

#define BUF_LEN 4096

#define BUF_FULL_INTERVAL BUF_LEN * int(1000000.0 / float(SAMPLE_RATE))

#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"

#define BUDDY_IP IPAddress(192, 168, 142, 105)
#define LISTEN_PORT 1592
