#include <Arduino.h>
#include <driver/i2s_std.h>
#include "i2scfg.h"

#ifndef MAX98357_h
#define MAX98357_h

struct MAX98357PinCfg {
    gpio_num_t bclk;
    gpio_num_t ws;
    gpio_num_t data;
    gpio_num_t enable;
};

class MAX98357 {
private:
    i2s_chan_handle_t chanHandle;
    bool enabled;
    MAX98357PinCfg pins;

public:
    MAX98357();
    bool begin(I2SCfg _cfg, MAX98357PinCfg _pins);
    bool end();
    void sleep();
    void wake();
    bool asleep();
    bool write(uint8_t* bytes, size_t byteCnt, size_t* bytesWritten);
};

#endif
