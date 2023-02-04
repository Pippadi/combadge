#include "driver/i2s.h"

#define MIC_WS 25
#define MIC_SD 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0
#define MIC_TIMER 0

#define SPK_WS 13
#define SPK_DATA 27
#define SPK_BCLK 12
#define SPK_PORT I2S_NUM_1

typedef int16_t sample_t;
#define BITS_PER_SAMPLE 16
#define BYTES_PER_SAMPLE BITS_PER_SAMPLE / 8

#define SAMPLE_RATE 8000
#define BUF_LEN 512
#define BUF_FULL_INTERVAL BUF_LEN * int(1000000.0 * (1.0 / float(SAMPLE_RATE)))

#define WIFI_SSID "MVWiFi"
#define WIFI_PASSWORD "A12168A17BC1519251521A18E219ECC"

#define BUDDY_IP IPAddress(192, 168, 142, 40)
#define UDP_PORT 1592
