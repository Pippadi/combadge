#include <driver/i2s.h>

#define BUF_LEN 1024
#define SAMPLE_RATE 8000
#define BITS_PER_SAMPLE 32

#define FREQ 440
#define PI 3.14

typedef int32_t sample_t;

sample_t buf[BUF_LEN];
i2s_port_t spkPort = I2S_NUM_1;

void setup() {
    Serial.begin(115200);

    const i2s_config_t spkCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(BITS_PER_SAMPLE),
        // .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = BUF_LEN,
    };

    i2s_pin_config_t spkPins = {
        .bck_io_num = 12,
        .ws_io_num = 13,
        .data_out_num = 27,
    };

    i2s_driver_install(spkPort, &spkCfg, 0, NULL);
    i2s_set_pin(spkPort, &spkPins);
}

void loop() {
    static int n = 0;
    for (int i=0; i<BUF_LEN; i++) {
        sample_t sample = f(n);
        buf[i] = sample;
        n++;
        // Serial.println(sample);
    }

    size_t bytesWritten;
    i2s_write(spkPort, (char*) buf, BUF_LEN*8, &bytesWritten, portMAX_DELAY);

    delayMicroseconds(1000000 * (double(BUF_LEN) / double(SAMPLE_RATE)));
}

sample_t f(int n) {
    double sinVal = sin((2.0 * PI * double(n*FREQ)) / double(SAMPLE_RATE));
    sample_t res = (pow(2, BITS_PER_SAMPLE) / 2) - 1;
    return sample_t(double(res) * sinVal);
}
