#include "driver/i2s.h"

#define MIC_WS 25
#define MIC_SD 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0
#define MIC_TIMER 0

#define SPK_WS 13
#define SPK_SD 27
#define SPK_BCLK 12
#define SPK_PORT I2S_NUM_1

#define SAMPLE_RATE 16000
#define BUF_LEN 256
#define BUF_FULL_INTERVAL BUF_LEN * int(1000000.0 * (1.0 / float(SAMPLE_RATE)))

// Changing this means changing data types in the rest of the code.
// This is just to make byte count to sample count conversions look prettier.
#define BYTES_PER_SAMPLE 2

#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"

#define BUDDY_IP IPAddress(192, 168, 142, 40)
#define UDP_PORT 1592
