#include <Arduino.h>
#include <driver/i2s.h>
#include "i2scfg.h"

#ifndef MAX98357_h
#define MAX98357_h

struct MAX98357PinCfg {
    int bclk;
    int ws;
    int data;
    int enable;
};

class MAX98357 {
    private:
    i2s_port_t port;
    bool enabled;
    MAX98357PinCfg pins;
    I2SCfg cfg;

    public:
    MAX98357();
    bool begin(i2s_port_t _port, I2SCfg _cfg, MAX98357PinCfg _pins);
    bool end();
    void sleep();
    void wake();
    bool asleep();
    bool write(uint8_t* bytes, size_t byteCnt, size_t* bytesWritten);
};

#endif
