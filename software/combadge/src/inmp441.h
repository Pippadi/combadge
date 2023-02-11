#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"

#ifndef INMP441_h
#define INMP441_h

struct INMP441PinCfg {
    int bclk;
    int ws;
    int data;
};

class INMP441 {
    private:
    i2s_port_t port;
    INMP441PinCfg pins;
    I2SCfg cfg;

    public:
    INMP441();
    bool begin(i2s_port_t _port, I2SCfg _cfg, INMP441PinCfg _pins);
    bool read(uint8_t* destination, size_t capacity, size_t* bytesRead);
};

#endif
