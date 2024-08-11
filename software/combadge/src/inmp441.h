#include <Arduino.h>
#include <driver/i2s_std.h>
#include "i2scfg.h"

#ifndef INMP441_h
#define INMP441_h

class INMP441 {
private:
    i2s_chan_handle_t chanHandle;

public:
    INMP441();
    bool begin(I2SCfg _cfg, MicPinCfg _pins);
    size_t read(int16_t* destination, size_t sampleCnt);
};

#endif
