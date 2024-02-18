# combadge

The code for the actual combadge, written in C++ for use with [Arduino](https://www.arduino.cc/), and of course the [ESP32 toolchain](https://github.com/espressif/arduino-esp32).
While the current hardware uses the ESP32-S3, the code aims to retain support for the ESP32.

## Usage

- Edit `config.h` to match your hardware and software configurations.
+ The default pins and peripherals correspond to the hardware in this repository. Change accordingly if using something different.
+ WiFi credentials and bridge host and port will need to be changed per your setup.
+ At present, all involved parties will need to have the same audio configuration. So, changing items such as `sample_t` or `SAMPLE_RATE` will require similar changes to the bridge and the other clients/combadges.
- Run `make SOC="esp32s3"` and `make flash PORT="/dev/<your port>"`. `SOC` can be set to `esp32` if required.

## Dependencies

- [Arduino CLI](https://arduino.github.io/arduino-cli/0.36/) (when building with Makefile)
- WiFi (See [`WiFiClient.h`](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiClient.h))
- I2S (See [`i2s.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/driver/include/driver/i2s.h) and [`hal/i2s_types.h`](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/include/hal/include/hal/i2s_types.h))

## Outline

At startup, two threads are started - one for transmitting microphone data and another for receiving and playing back speaker data.

All data is sent with the Combadge Real-time Audio Protocol (CRAP).

The intricacies of I2S communication are abstracted out to classes, specially defined for each hardware peripheral.
The underlying plan is to eventually define a speaker and microphone interface, and write uniform implementations for specific microphone and speaker peripherals to make switching hardware simpler.

Configurations, constants, and other definitions can be found in [`config.h`](config.h).

## Combadge Real-time Audio Protocol

[CRAP](src/crap.h) is a standard for sending audio data and transmission-related events.

### Topology

A central bridge handles all communication to an from an individual client (badge).
Clients are aware only of the existence of the bridge.
The bridge has the responsibility of routing audio data between clients, and sending the appropriate ephemeral events when necessary.

### Communication

CRAP uses TCP as the underlying communication protocol. A client connects to a bridge on startup, and keeps the connection open.
The client is considered to be offline when the socket is closed.

Information is sent in CRAP packets. A packet mandatorily has a header, and may have a body.
The `PacketHeader` has the following structure.

```c
typedef struct {
    uint32_t type;
    uint32_t size;
} PacketHeader;
```

`type` holds an enumerated intent of the message. `size` holds the size in bytes of the rest of the message body.
Currently, three packet `type`s have been defined.
These three messages are used both by badges and the bridge to start and stop transmissions.

- `AUDIO_START`: Starts an audio transmission. `size` is 0 as no further body is necessary.
- `AUDIO_STOP`: Ends an audio transmission. `size` is 0 as no further body is necessary.
- `AUDIO_DATA`: Holds audio samples in the agreed-upon format (currently raw signed 16-bit little-endian). `size` is the number of samples read times the configured `BYTES_PER_SAMPLE`.

Right now, configurations like sample rate and bitness are hardcoded (but modifiable in `config.h`) for the sake of simplicity.
Allowing on-the-fly setting of such parameters is a goal for the future.
It is currently assumed that both the bridge and clients have been appropriately configured to use the same audio format.

For more information on the bridge, see [/software/bridge](/software/bridge).
