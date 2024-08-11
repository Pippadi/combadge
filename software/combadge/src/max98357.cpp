#include <Arduino.h>
#include <driver/i2s_std.h>
#include "i2scfg.h"
#include "max98357.h"

MAX98357::MAX98357() {}

bool MAX98357::begin(I2SCfg _cfg, MAX98357PinCfg _pins) {
    pins = _pins;

    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    i2s_new_channel(&chanCfg, &chanHandle, NULL);

    i2s_std_config_t spkCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_cfg.sampleRate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = _pins.bclk,
            .ws = _pins.ws,
            .dout = _pins.data,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    i2s_channel_init_std_mode(chanHandle, &spkCfg);
    if (i2s_channel_enable(chanHandle) != ESP_OK) {
        return false;
    }

    pinMode(pins.enable, OUTPUT);
    MAX98357::sleep();

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

bool MAX98357::asleep() {
    return !enabled;
}

bool MAX98357::write(uint8_t* bytes, size_t byteCnt, size_t* bytesWritten) {
    esp_err_t err = i2s_channel_write(chanHandle, bytes, byteCnt, bytesWritten, portMAX_DELAY);
    if (err != ESP_OK) {
        return false;
    }
    return true;
}

bool MAX98357::end() {
    i2s_channel_disable(chanHandle);
    return i2s_del_channel(chanHandle) == ESP_OK;
}
