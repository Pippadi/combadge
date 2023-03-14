# Converting sound clips

In this project, sound clips generally reside as header files with large arrays of data points.

First, ensure that your clip is adequately loud and well-cropped with a tool like [Audacity](https://www.audacityteam.org).

Next, check the sample rate in your [`config.h`](/software/combadge/config.h) in the `SAMPLE_RATE` definition.
Also note the bitness defined with `sample_t` (e.g. `int16_t` is 16-bit).

Finally, invoke `HeaderFromSound.py` to generate the header file accordingly.

```
HeaderFromSound.py [-h] [--rate SAMPLERATE] [--bitness BITNESS] [--name CLIPNAME] INPUTFILE
```

Here's an example:
```sh
python3 /path/to/HeaderFromSound.py --bitness 16 --rate 44100 --name Chirp > Chirp.h
```

Move the generated header file to the appropriate [location](/software/combadge/sounds) and use.
