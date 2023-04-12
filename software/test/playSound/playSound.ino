#include <driver/i2s.h>
#include "TNGChirp1.h"

#define BUF_LEN 1024
#define BITS_PER_SAMPLE 16
#define BYTES_PER_SAMPLE BITS_PER_SAMPLE / 8
#define SAMPLE_RATE 44100

#define DATA_PIN 27
#define BCLK_PIN 12
#define WS_PIN 13

i2s_port_t spkPort = I2S_NUM_1;

void setup() {
    Serial.begin(115200);

    const i2s_config_t spkCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(BITS_PER_SAMPLE),

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

    playSound(TNGChirp1, TNGChirp1SizeBytes);

    i2s_driver_uninstall(spkPort);
}

void playSound(const int16_t* sound, const size_t soundSizeBytes) {
    size_t bytesWritten;
    i2s_write(spkPort, (char*) sound, soundSizeBytes, &bytesWritten, portMAX_DELAY);
}

void loop() {}
