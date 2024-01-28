#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"
#include "sph0645.h"

SPH0645::SPH0645() {}

bool SPH0645::begin(i2s_port_t _port, I2SCfg _cfg, MicPinCfg _pins) {
    port = _port;
    cfg = _cfg;
    pins = _pins;

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = cfg.sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
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

#ifdef SOC_ESP32_S3
    REG_SET_BIT(I2S_RX_TIMING_REG(port), BIT(1));
    REG_SET_BIT(I2S_RX_CONF1_REG(port), I2S_RX_MSB_SHIFT);
#endif

    err = i2s_set_pin(port, &micPins);
    return err == ESP_OK;
}

// Accepts 16 bit buffers only
size_t SPH0645::read(int16_t* destination, size_t sampleCnt) {
    size_t bytesRead;
    int32_t temp[sampleCnt];
    size_t samplesRead;

    esp_err_t err = i2s_read(port, (uint8_t*) temp, sampleCnt*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        return 0;
    }

    samplesRead = bytesRead / sizeof(int32_t);
    for (int i=0; i<samplesRead; i++) {
        // Discard unused lower 12 bits. Sign bit is already where it needs to be.
        temp[i] >>= 12;
        destination[i] = (int16_t) (temp[i] & 0xFFFF);
    }
    return samplesRead;
}
