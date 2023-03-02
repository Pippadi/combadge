# Component Choices

The combadge is built around an [ESP32-WROOM-32](https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=54&7&8&8&2&3https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=54&7&8&8&2&3).

It is supposedly not recommended for new designs, but I chose it regardless because:

- It is the wireless board with which I am most familiar
- It has hardware I2S and touch support
- It is well-supported by the Arduino toolchain
- It is cheaply available in my region

## Sound

I've chosen to adopt I2S peripherals for this project, as they promise better audio quality than their analog counterparts, while reducing the amount of analog circuitry I have to design.
The [ESP32](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/api-reference/peripherals/i2s.html) supports two I2S peripherals in hardware.

### Microphone

I've zeroed in on the [CMM-3526D-261-I2S-TR](https://www.cuidevices.com/product/audio/microphones/mems-microphones/cmm-3526d-261-i2s-tr).
I suppose any similar microphone will do the job.

For testing, I'm using the INMP441, because it's available as a module. I would have stuck with it for the PCB, but it seems to be discontinued by the manufacturer.

### Speaker

With a speaker must come an amplifier. I chose the [MAX98357AEWL+T](https://www.maximintegrated.com/en/products/analog/audio/MAX98357A.html) because it's a class D amplifier that takes I2S input, and can drive a speaker with pretty much no external circuitry.
It's also what I used to prototype.

I'm still on the lookout for an actual speaker.

## Power Management

The unit runs on a lithium-polymer battery. Exact model and size undecided as of now. It will be one of the usual 3.7V LiPo cells.
At this stage, though, I'm more concerned with battery management.

All the chosen components of the combadge operate on 3.3V. I was hoping to find an all-in-one charging and voltage-regulating IC, but my searches yielded none for these parameters.
For charging, I plan to use an [MCP73831](https://www.microchip.com/en-us/product/MCP73831) and a zener diode circuit for over-discharge protection.
I found the super tiny 250mA [NCP160](https://www.onsemi.com/products/power-management/linear-regulators-ldo/ncp160) LDO linear regulator for 3.3V power.

I'm hoping decoupling capacitors will take care of any current spikes the ESP32 and speakers may need.

# PCB

The PCB has been designed in KiCAD 6.

- Footprints are chosen to be as large as possible where applicable, to facilitate hand-soldering.
- All the QFN/DFN-type ICs are on one side of the PCB so that there is some hope of reflowing them on at home.
- The PCB has headers to which battery and speaker wires can be soldered.

To load this KiCAD project, you will need to get the following third-party symbols and footprints:

- [CSS-1210TB](https://app.ultralibrarian.com/details/711a64f8-0773-11ed-b159-0a34d6323d74/Nidec-Copal-Electronics/CSS-1210TB?uid=38990419&exports=KiCAD&open=exports)
- [RN4989](https://vendor.ultralibrarian.com/toshiba/Embedded?vdrPN=RN4989#)
- [MAX98357AEWL+T](https://vendor.ultralibrarian.com/Maxim/Embedded?vdrPN=MAX98357AEWL%2BT)
- [CMM-3526D-261-I2S-TR](https://www.cuidevices.com/product/resource/pcbfootprint/cmm-3526d-261-i2s-tr)

You will need to change the paths to the third party symbols in the library and footprint managers.
