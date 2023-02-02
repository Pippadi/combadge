#include <driver/i2s.h>

#define BUF_LEN 1024
#define SAMPLE_RATE 8000
#define BITS_PER_SAMPLE 16
#define BITS_PER_BYTE 8

#define FREQ 1000
#define PI 3.1415

#define DATA_PIN 27
#define BCLK_PIN 12
#define WS_PIN 13

typedef int16_t sample_t;

// Uncomment such lines if using I2S_CHANNEL_FMT_RIGHT_LEFT below
// sample_t buf[BUF_LEN*2];
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

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUF_LEN,
    };

    i2s_pin_config_t spkPins = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = DATA_PIN,
    };

    esp_err_t err;
    err = i2s_driver_install(spkPort, &spkCfg, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("Error installing driver");
        Serial.println(err);
        while(true);
    }
    err = i2s_set_pin(spkPort, &spkPins);
    if (err != ESP_OK) {
        Serial.println("Error setting pins");
        Serial.println(err);
        while(true);
    }
}

void loop() {
    static uint64_t n = 0;
    for (int i=0; i<BUF_LEN; i++) {
        sample_t sample = f(n);

        // buf[i*2] = sample;
        // buf[(i*2)+1] = sample;
        buf[i] = sample;

        n++;
    }

    size_t bytesWritten;
    // i2s_write(spkPort, (char*) buf, BUF_LEN*2*BITS_PER_SAMPLE/BITS_PER_BYTE, &bytesWritten, portMAX_DELAY);
    i2s_write(spkPort, (char*) buf, BUF_LEN*BITS_PER_SAMPLE/BITS_PER_BYTE, &bytesWritten, portMAX_DELAY);
}

sample_t f(uint64_t n) {
    double sinVal = sin((2.0 * PI * double(n*FREQ)) / double(SAMPLE_RATE));
    sample_t res = pow(2, BITS_PER_SAMPLE-1);
    return sample_t(double(res) * sinVal);
}
