#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"
#include "inmp441.h"

#define INT16MAX 0x7FFF

INMP441::INMP441() {}

bool INMP441::begin(i2s_port_t _port, I2SCfg _cfg, INMP441PinCfg _pins) {
    port = _port;
    cfg = _cfg;
    pins = _pins;

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = cfg.sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // L/R to GND

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t micPins = {
        .bck_io_num = pins.bclk,
        .ws_io_num = pins.ws,
        .data_in_num = pins.data,
    };

    esp_err_t err;
    err = i2s_driver_install(port, &micCfg, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }
    err = i2s_set_pin(port, &micPins);
    return err == ESP_OK;
}

// Accepts 16 bit buffers only
size_t INMP441::read(int16_t* destination, size_t sampleCnt) {
    size_t bytesRead;
    int32_t temp[sampleCnt];

    esp_err_t err = i2s_read(port, (uint8_t*) temp, sampleCnt*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        return 0;
    }

    for (int i=0; i<sampleCnt; i++) {
        // Helpful:
        // - https://esp32.com/viewtopic.php?t=15185
        // - https://github.com/atomic14/esp32-walkie-talkie/blob/main/lib/audio_input/src/I2SMEMSSampler.cpp
        // Discard unused lower 8 bits, and get rid of 3 bits of noise.
        // The number 11 was empirically determined to provide the best signal.
        temp[i] >>= 11;
        destination[i] = (int16_t) temp[i];
    }
    return bytesRead / sizeof(int16_t);
}
