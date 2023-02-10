#!/usr/bin/python3

import sys

if len(sys.argv) < 3:
    print("""Generate header file for sound clip\n./headerFromRaw.py <Raw Sound File> <Name of clip>""")
    exit(1)

infile = sys.argv[1]
soundName = sys.argv[2]

print("const PROGMEM sample_t %s[] = {" % soundName)

with open(infile, "rb") as file:
    contents = file.read()
    for i in range(1, len(contents), 2):
        print("0x%04x, "  % ((contents[i]<<8) + contents[i-1]), end="")
        if (i+1) % 16 == 0:
            print()

print("\n};\n\nconst size_t %sSizeBytes = sizeof(%s);" % (soundName, soundName))
