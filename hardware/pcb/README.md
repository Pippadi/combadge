# Electronics and PCB

See the [assets](assets) directory for screenshots of the schematics and PCB.

## Component Choices

The combadge is built around an [ESP32-S3-WROOM-1](https://www.espressif.com/en/support/download/documents/modules?keys=&field_type_tid%5B%5D=838).

I chose it because:

- It has hardware I2S and touch support
- It is well-supported by the Arduino toolchain
- It is cheaply available in my region

### Sound

I've chosen to adopt I2S peripherals for this project, as they promise better audio quality than their analog counterparts, while reducing the amount of analog circuitry I have to design.
The [ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2s.html) supports two I2S peripherals in hardware.

#### Microphone

I've zeroed in on the [Knowles SPH0645LM4H-1](https://www.knowles.com/docs/default-source/default-document-library/sph0645lm4h-1-datasheet.pdf?Status=Master&sfvrsn=751076b1_2).
Any similar microphone ought to do the job.

I use the INMP441 for prototyping because it's available as a module. I would have stuck with it for the PCB, but it seems to be discontinued by the manufacturer.

#### Speaker

The [Taoglas SPKM.10.8.A](https://www.taoglas.com/product/10-mm-round-miniature-speaker-500mw/) seems like an adequate speaker for the job.

A [MAX98357AETE+T](https://www.analog.com/en/products/max98357a.html#product-overview) class D amplifier drives the speaker.
It's what I use to prototype.

### Power Management

The unit runs on a lithium-polymer battery. Exact model and size undecided as of now. It will be one of the usual 3.7V LiPo cells.
All the chosen components of the combadge operate on 3.3V. I was hoping to find an all-in-one charging and voltage-regulating IC, but my searches yielded none for these parameters.

Charging is handled by an [MCP73831](https://www.microchip.com/en-us/product/MCP73831).
A 300mA [TLV74333PDBVR](https://www.ti.com/product/TLV743P/part-details/TLV74333PDBVR) LDO linear regulator provides 3.3V power.
A [TLV431AQFTA](https://www.diodes.com/assets/Datasheets/TLV431Q.pdf)-based circuit conditionally disables the regulator for battery over-discharge protection.

## PCB

The PCB has been designed in KiCAD.

![PCB](assets/pcb.png)

- Footprints are chosen to be as large as possible where applicable, to facilitate hand-soldering or rework.
- The PCB has headers to which battery and speaker wires can be soldered.
- Certain measurements can be seen in the `User.Comments` layer.

This project makes use of the official Espressif [ESP32 KiCAD libraries](https://github.com/espressif/kicad-libraries).

You will need to change the paths to the third party symbols in the library and footprint managers. I've just plonked everything in a gitignored folder called `third-party`.

## Notes

### Crackling on speaker

When I was prototyping, I was powering everything off my computer's USB port. The INMP441 microphone module worked well, but the MAX98357 with no power supply filtering was horribly noisy.
Some beefy electrolytic capacitors reduced the noise, but did not eliminate it.

The root cause of the noise turned out to be the fact that I was powering the speaker through the AMS1117 3.3V regulator on my ESP32 dev board.
The regulator could not handle the current spikes demanded by the speaker, so its output voltage ended up dropping.
Hooking the speaker up directly to the USB 5V line (VIN on my particular dev board) solved the issue.

The PCB has been designed such that the amplifier is powered directly by the battery.
There are also some large low-transient-response-time ceramic capacitors for any bursts of energy components may need.
