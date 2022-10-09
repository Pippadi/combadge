# Electronics and PCB

The combadge is built around an [ESP32-WROOM-32](https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=54&7&8&8&2&3https://www.espressif.com/en/support/documents/technical-documents?keys=&field_type_tid%5B%5D=54&7&8&8&2&3).
It is supposedly not recommended for new designs, but it's among the best supported by Arduino for programming, and it seems to be the only ESP32 variant that is cheaply available in my region.

## Sound

I've chosen to adopt I2S peripherals for this project, as they promise better audio quality than their analog counterparts, while reducing the amount of analog circuitry I have to design.
The [ESP32](https://docs.espressif.com/projects/esp-idf/en/v4.2.3/esp32/api-reference/peripherals/i2s.html) supports two I2S peripherals in hardware.

### Microphone

I've zeroed in on the [CMM-3526D-261-I2S-TR](https://www.cuidevices.com/product/audio/microphones/mems-microphones/cmm-3526d-261-i2s-tr).
I suppose any similar microphone will do the job. The manufacturer has very kindly provided [an official footprint](https://www.cuidevices.com/product/resource/pcbfootprint/cmm-3526d-261-i2s-tr).

### Speaker

With a speaker must come an amplifier. I chose the [MAX98357AEWL+T](https://www.mouser.in/datasheet/2/256/MAX98357A-MAX98357B-271244.pdf) because it's a class D amplifier that takes I2S input, and can drive a speaker with pretty much no external circuitry.
It's also what I used to prototype.

Its footprint can be found on its [product page](https://www.maximintegrated.com/en/products/analog/audio/MAX98357A.html).

I'm still on the lookout for an actual speaker.

## Power Management

The unit runs on a lithium-polymer battery. Exact model and size unknown as of now. I'm expecting it to be one of the usual 3.7V LiPo cells.
At this stage, though, I'm more concerned with battery management.

All the chosen components of the combadge operate on 3.3V. I was hoping to find an all-in-one charging and voltage-regulating IC, but none seem to exist for these parameters.
For charging, I plan to use an [XC6808A4D28R-G](https://www.torexsemi.com/products/battery-charge-ics/series/?name=xc6808) and a zener diode circuit for over-discharge protection.
I found the super tiny 250mA [NCP160](https://www.onsemi.com/products/power-management/linear-regulators-ldo/ncp160) LDO linear regulator for 3.3V power.

I still need to verify whether 250mA is enough for the entire combadge (it better be; it's running off a battery).
I'm hoping decoupling capacitors will take care of any current spikes the ESP32 and speakers may need.

You'll need the symbol and PCB footprint for the [Si2342DS](https://www.vishay.com/en/product/63302/).

## The rest

Major components used are more-or-less finalized. PCB footprints are chosen to be as large as possible where applicable, to facilitate hand-soldering.
Both the battery and speaker are assumed to be soldered to the board with wires.
