#include <driver/i2s.h>

typedef int32_t sample_t;

#define BYTES_PER_SAMPLE sizeof(sample_t)
#define BITS_PER_SAMPLE BYTES_PER_SAMPLE * 8
#define BUF_LEN 256
#define SAMPLE_RATE 44100

#define DATA_PIN 33
#define BCLK_PIN 32
#define WS_PIN 25
#define PORT I2S_NUM_0

void setup() {
    Serial.begin(115200);

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(BITS_PER_SAMPLE),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // L/R to GND

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t micPins = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = -1,
        .data_in_num = DATA_PIN,
    };

    esp_err_t err;
    err = i2s_driver_install(PORT, &micCfg, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("I2S driver initialization failed");
        while (true);
    }
    err = i2s_set_pin(PORT, &micPins);
    if (err != ESP_OK) {
        Serial.println("Setting pins failed");
        while (true);
    }
}

void loop() {
    static sample_t buf[BUF_LEN];
    size_t bytesRead;
    esp_err_t err = i2s_read(PORT, (uint8_t*) buf, BUF_LEN*BYTES_PER_SAMPLE, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("Error reading from microphone");
        return;
    }
    for (int i = 0; i<bytesRead/BYTES_PER_SAMPLE; i++) {
        Serial.println(buf[i] >> 8); // Data is only in the high 24 bits
    }
}
