#define MIC_WS 25
#define MIC_SD 33
#define MIC_BCLK 32
#define MIC_PORT I2S_NUM_0
#define MIC_TIMER 0

#define SAMPLE_RATE 44100
#define BUF_LEN 256
#define BUF_FULL_INTERVAL BUF_LEN * int(1000000.0 * (1.0 / float(44100)))

#define BUDDY_IP IPAddress(192, 168, 142, 40)
#define UDP_PORT 1592
