#include <Arduino.h>
#include <driver/i2s_std.h>
#include "i2scfg.h"
#include "inmp441.h"

INMP441::INMP441() {}

bool INMP441::begin(I2SCfg _cfg, MicPinCfg _pins) {
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    i2s_new_channel(&chanCfg, NULL, &chanHandle);

    const i2s_std_config_t micCfg = {
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

    i2s_channel_init_std_mode(chanHandle, &micCfg);
    return i2s_channel_enable(chanHandle) == ESP_OK;
}

// Accepts 16 bit buffers only
size_t INMP441::read(int16_t* destination, size_t sampleCnt) {
    size_t bytesRead;
    int32_t temp[sampleCnt];
    size_t samplesRead;

    esp_err_t err = i2s_channel_read(chanHandle, (uint8_t*) temp, sampleCnt*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        return 0;
    }

    samplesRead = bytesRead / sizeof(int32_t);
    for (int i=0; i<samplesRead; i++) {
        // Helpful:
        // - https://esp32.com/viewtopic.php?t=15185
        // - https://github.com/atomic14/esp32-walkie-talkie/blob/main/lib/audio_input/src/I2SMEMSSampler.cpp
        // Discard unused lower 8 bits, and get rid of 3 bits of noise.
        // The number 11 was empirically determined to provide the best signal.
        temp[i] >>= 11;
        temp[i] &= 0xFFFF; // Truncating loud sounds
        destination[i] = (int16_t) temp[i];
    }
    return samplesRead;
}
