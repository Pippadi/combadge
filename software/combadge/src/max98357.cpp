#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"
#include "max98357.h"

MAX98357::MAX98357() {}

bool MAX98357::begin(i2s_port_t _port, I2SCfg _cfg, MAX98357PinCfg _pins) {
    port = _port;
    cfg = _cfg;
    pins = _pins;

    const i2s_config_t spkCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = cfg.sampleRate,
        .bits_per_sample = i2s_bits_per_sample_t(cfg.bitsPerSample),
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = cfg.bufLen,
    };

    i2s_pin_config_t spkPins = {
        .bck_io_num = pins.bclk,
        .ws_io_num = pins.ws,
        .data_out_num = pins.data,
    };

    esp_err_t err;
    err = i2s_driver_install(port, &spkCfg, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }
    err = i2s_set_pin(port, &spkPins);
    if (err != ESP_OK) {
        return false;
    }

    pinMode(pins.enable, OUTPUT);
    sleep();

    return true;
}

void MAX98357::wake() {
    enabled = true;
    digitalWrite(pins.enable, HIGH);
    delayMicroseconds(10);
}

void MAX98357::sleep() {
    enabled = false;
    digitalWrite(pins.enable, LOW);
}

bool MAX98357::write(uint8_t* bytes, size_t byteCnt, size_t* bytesWritten) {
    esp_err_t err = i2s_write(port, bytes, byteCnt, bytesWritten, portMAX_DELAY);
    if (err != ESP_OK) {
        return false;
    }
    return true;
}
