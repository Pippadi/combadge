#include <Arduino.h>
#include <driver/i2s_std.h>
#include <soc/i2s_reg.h>
#include "i2scfg.h"
#include "sph0645.h"

SPH0645::SPH0645() {}

bool SPH0645::begin(i2s_port_t port, I2SCfg _cfg, MicPinCfg _pins) {
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(port, I2S_ROLE_MASTER);
    i2s_new_channel(&chanCfg, NULL, &chanHandle);

    i2s_std_config_t micCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_cfg.sampleRate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = _pins.bclk,
            .ws = _pins.ws,
            .dout = I2S_GPIO_UNUSED,
            .din = _pins.data,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }
        }
    };

    micCfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    i2s_channel_init_std_mode(chanHandle, &micCfg);
    esp_err_t err = i2s_channel_enable(chanHandle);
    if (err != ESP_OK) {
        return false;
    }

    REG_SET_BIT(I2S_RX_TIMING_REG(port), BIT(1));
    REG_SET_BIT(I2S_RX_CONF1_REG(port), I2S_RX_MSB_SHIFT);

    return true;
}

// Accepts 16 bit buffers only
size_t SPH0645::read(int16_t* destination, size_t sampleCnt) {
    size_t bytesRead;
    int32_t temp[sampleCnt];
    size_t samplesRead;

    esp_err_t err = i2s_channel_read(chanHandle, (uint8_t*) temp, sampleCnt*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        return 0;
    }

    samplesRead = bytesRead / sizeof(int32_t);
    for (int i=0; i<samplesRead; i++) {
        // Discard unused lower 11 bits. Sign bit is already where it needs to be.
        temp[i] >>= 11;
        destination[i] = (int16_t) (temp[i] & 0xFFFF);
    }
    return samplesRead;
}
