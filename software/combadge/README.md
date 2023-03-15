# combadge

The code for the actual combadge, written in C++ for use with [Arduino](https://www.arduino.cc/), and of course the [ESP32 toolchain](https://github.com/espressif/arduino-esp32).

## Dependencies

- I2S (See [`i2s.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/driver/include/driver/i2s.h) and [`hal/i2s_types.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/hal/include/hal/i2s_types.h))

## Outline

Right now, at startup, two threads are started - one for transmitting microphone data and another for receiving and playing back speaker data.

All data is sent over TCP.
For simplicity's sake, only peer-to-peer communication is performed with `BUDDY_IP` as defined in `config.h`.
In time, sound data will be relayed through a central server, which will aggregate multiple sound streams from multiple clients and provide a single data stream for each client.
There is as yet no protocol defined for sending data between combadges.

The intricacies of I2S communication are abstracted out to classes, specially defined for each hardware peripheral.
The underlying plan is to define a speaker and microphone interface, and write uniform implementations for specific microphone and speaker peripherals to make switching hardware simpler.
