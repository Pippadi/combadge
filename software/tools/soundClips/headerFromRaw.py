#!/usr/bin/python3

import sys
import math

if len(sys.argv) < 4:
    print("""Generate header file for sound clip\n./headerFromRaw.py <Raw Sound File> <Name of clip> <Bits per sample>""")
    exit(1)

infile = sys.argv[1]
soundName = sys.argv[2]
bitsPerSample = int(sys.argv[3])
bytesPerSample = bitsPerSample // 8
formatSpecifier = "0x%%0%dx, " % (bitsPerSample/4)

print("const PROGMEM sample_t %s[] = {" % soundName)

with open(infile, "rb") as file:
    contents = file.read()
    for i in range(0, len(contents)//bytesPerSample):
        sample = 0
        for j in range(0, bytesPerSample):
            sample = (sample<<8) + contents[(i*bytesPerSample)+j]
        print(formatSpecifier % sample, end="")
        if (i+1) % 16 == 0:
            print('\n\t', end="")

print("\n};\n\nconst size_t %sSizeBytes = sizeof(%s);" % (soundName, soundName))
