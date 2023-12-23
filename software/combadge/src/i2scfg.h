#include <Arduino.h>
#include <driver/i2s.h>

#ifndef I2SCfg_h
#define I2SCfg_h

struct I2SCfg {
    int sampleRate;
    int bitsPerSample;
};

struct MicPinCfg {
    int bclk;
    int ws;
    int data;
};

#endif
