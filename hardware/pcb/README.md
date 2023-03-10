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

With a speaker must come an amplifier. I chose the [MAX98357AETE+T](https://www.analog.com/en/products/max98357a.html#product-overview) because it's a class D amplifier that takes I2S input, and can drive a speaker with pretty much no external circuitry.
It's also what I used to prototype.

The [Taoglas SPKM.10.8.A](https://www.taoglas.com/product/10-mm-round-miniature-speaker-500mw/) seems like an adequate speaker for the job.

## Power Management

The unit runs on a lithium-polymer battery. Exact model and size undecided as of now. It will be one of the usual 3.7V LiPo cells.
At this stage, though, I'm more concerned with battery management.

All the chosen components of the combadge operate on 3.3V. I was hoping to find an all-in-one charging and voltage-regulating IC, but my searches yielded none for these parameters.
For charging, I plan to use an [MCP73831](https://www.microchip.com/en-us/product/MCP73831) and a [TLV431AQFTA](https://www.diodes.com/assets/Datasheets/TLV431Q.pdf)-based circuit for over-discharge protection.
I found the 300mA [TLV74333PDBVR](https://www.ti.com/product/TLV743P/part-details/TLV74333PDBVR) LDO linear regulator for 3.3V power.

I'm hoping decoupling capacitors will take care of any current spikes the ESP32 and speakers may need.

# PCB

The PCB has been designed in KiCAD.

![PCB](/assets/pcb.png)

- Footprints are chosen to be as large as possible where applicable, to facilitate hand-soldering.
- The PCB has headers to which battery and speaker wires can be soldered.
- Some symbols used have part numbers slightly different from the ones specified in this document. They've been used because they are first-party KiCAD footprints, and they have matching pin configuration. It's just a pain hunting for symbols online.

To load this KiCAD project, you will need to get the following third-party symbols and footprints:

- [CSS-1210TB](https://app.ultralibrarian.com/details/711a64f8-0773-11ed-b159-0a34d6323d74/Nidec-Copal-Electronics/CSS-1210TB?uid=38990419&exports=KiCAD&open=exports)
- [MAX98357AETE+T](https://vendor.ultralibrarian.com/ADI/embedded?mfrName=Maxim%20Integrated%20Products&mfrpn=MAX98357AETE%2bT)
- [CMM-3526D-261-I2S-TR](https://www.cuidevices.com/product/resource/pcbfootprint/cmm-3526d-261-i2s-tr)

You will need to change the paths to the third party symbols in the library and footprint managers. I've just plonked everything in a gitignored folder called `third-party`.
