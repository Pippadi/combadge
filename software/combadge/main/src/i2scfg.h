#include <Arduino.h>
#include <driver/i2s_std.h>

#ifndef I2SCfg_h
#define I2SCfg_h

struct I2SCfg {
    int sampleRate;
    int bitsPerSample;
};

struct MicPinCfg {
    gpio_num_t bclk;
    gpio_num_t ws;
    gpio_num_t data;
};

#endif
