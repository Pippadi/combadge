#include "I2S.h"

#define BUF_LEN 2048
#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 32

#define FREQ 100
#define PI 3.14

uint32_t buf[BUF_LEN];

void setup() {
    Serial.begin(115200);
    I2S.begin(I2S_PHILIPS_MODE, SAMPLE_RATE, BITS_PER_SAMPLE);
    I2S.setDataPin(27);
    I2S.setFsPin(13);
    I2S.setSckPin(12);
}

void loop() {
    static int n = 0;
    for (int i=0; i<BUF_LEN; i++) {
        uint32_t sample = f(n);
        buf[i] = sample;
        n++;
        // Serial.println(sample);
    }
    I2S.write(buf, BUF_LEN);
    delayMicroseconds(1000000 * (double(BUF_LEN) / double(SAMPLE_RATE)));
}

uint32_t f(int n) {
    double sinVal = sin((2.0 * PI * double(n*FREQ)) / double(SAMPLE_RATE));
    uint32_t res = pow(2, BITS_PER_SAMPLE);
    return (res / 2) + uint32_t(double(res) * sinVal);
}
