# combadge

The code for the actual combadge, written in C++ for use with [Arduino](https://www.arduino.cc/), and of course the [ESP32 toolchain](https://github.com/espressif/arduino-esp32).

## Dependencies

- I2S (See [`i2s.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/driver/include/driver/i2s.h) and [`hal/i2s_types.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/hal/include/hal/i2s_types.h))

## Outline

Right now, at startup, two threads are started - one for transmitting microphone data and another for receiving and playing back speaker data.

All data is sent over TCP.
For simplicity's sake, only peer-to-peer communication is performed with `BUDDY_IP` as defined in `config.h`.
Each combdage listens on `LISTEN_PORT` for its buddy, and when tapped, connects as a client to its buddy.
Samples are sent raw in packets of size `BUF_LEN * BYTES_PER_SAMPLE`.
Playback begins when the buddy connects, and ends when it disconnects.

In time, sound data will be encoded and then relayed through a central server, which will aggregate multiple sound streams from multiple clients and provide a single data stream for each client.

The intricacies of I2S communication are abstracted out to classes, specially defined for each hardware peripheral.
The underlying plan is to eventually define a speaker and microphone interface, and write uniform implementations for specific microphone and speaker peripherals to make switching hardware simpler.

Right now, configurations like sample rate and bitness are hardcoded (but modifiable in `config.h`) for the sake of simplicity.
Allowing on-the-fly setting of such parameters is a goal for the future.
