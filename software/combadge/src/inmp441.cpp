#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"
#include "inmp441.h"

INMP441::INMP441() {}

bool INMP441::begin(i2s_port_t _port, I2SCfg _cfg, INMP441PinCfg _pins) {
    port = _port;
    cfg = _cfg;
    pins = _pins;

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = cfg.sampleRate,
        .bits_per_sample = i2s_bits_per_sample_t(cfg.bitsPerSample),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = cfg.bufLen,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t micPins = {
        .bck_io_num = pins.bclk,
        .ws_io_num = pins.ws,
        .data_out_num = pins.data,
    };

    esp_err_t err;
    err = i2s_driver_install(port, &micCfg, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }
    err = i2s_set_pin(port, &micPins);
    return err == ESP_OK;
}

bool INMP441::read(uint8_t* destination, size_t capacity, size_t* bytesRead) {
    esp_err_t err = i2s_read(port, destination, capacity, bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        return false;
    }
    return true;
}
