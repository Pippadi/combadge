#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"

#ifndef SPH0645_h
#define SPH0645_h

class SPH0645 {
    private:
    i2s_port_t port;
    MicPinCfg pins;
    I2SCfg cfg;

    public:
    SPH0645();
    bool begin(i2s_port_t _port, I2SCfg _cfg, MicPinCfg _pins);
    size_t read(int16_t* destination, size_t sampleCnt);
};

#endif
