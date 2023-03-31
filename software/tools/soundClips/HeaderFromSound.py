#!/usr/bin/python3

import os
import math
import argparse
import subprocess

parser = argparse.ArgumentParser(description="Generate C header for sound clip")

parser.add_argument(dest="inputfile", metavar="INPUTFILE", nargs=1, help="Input audio file")
parser.add_argument("--rate", dest="samplerate", type=int, help="Sample rate", default=44100)
parser.add_argument("--bitness", dest="bitness", type=int, help="Bitness", default=16)
parser.add_argument("--name", dest="clipname", help="Clip name", default="Clip")

args = parser.parse_args()

bytesPerSample = args.bitness // 8
formatSpecifier = "0x%%0%dx, " % (args.bitness/4)
tempfile = "tmp" + args.clipname + ".raw"

# Example ffmpeg command:
# ffmpeg -i HailBeep.mp3 -f s16be -ac 1 -ar 44100 HailBeep.raw
subprocess.run([
    "ffmpeg", 
    "-i", 
    args.inputfile[0], 
    "-f", 
    "s{}be".format(args.bitness), 
    "-ac", 
    "1", 
    "-ar", 
    str(args.samplerate), 
    tempfile,
])

print("const PROGMEM sample_t %s[] = {\n\t" % args.clipname, end="")

with open(tempfile, "rb") as file:
    contents = file.read()
    for i in range(0, len(contents)//bytesPerSample):
        sample = 0
        for j in range(0, bytesPerSample):
            sample = (sample<<8) + contents[(i*bytesPerSample)+j]
        print(formatSpecifier % sample, end="")
        if (i+1) % 16 == 0:
            print('\n\t', end="")

print("\n};\n\nconst size_t %sSizeBytes = sizeof(%s);" % (args.clipname, args.clipname))

os.remove(tempfile)
