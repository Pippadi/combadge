#!/usr/bin/python3

import sys
import socket
import time
import struct

PORT = 1701


class CRAPMsgType:
    AUDIO_START = 1
    AUDIO_STOP = 2
    AUDIO_DATA = 3


class Header:
    def __init__(self, t=0, s=0):
        self.type = t
        self.size = s

    def asBytes(self):
        return struct.pack("<ll", self.type, self.size)

    def fromBytes(self, b):
        self.type, self.size = struct.unpack("<ll", b)

    def __len__(self):
        return 8  # Two 32-bit ints


if len(sys.argv) < 2:
    print("USAGE: crapRecorder.py <Output file>")
    exit(1)

soundFileIn = sys.argv[1]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("0.0.0.0", PORT))
sock.listen()

print("Waiting for connection...")

conn, addr = sock.accept()

print("Connected to", addr)

with conn:
    f = open(soundFileIn, "wb")
    shouldReceive = True
    while shouldReceive:
        h = Header()
        h.fromBytes(conn.recv(len(h)))
        print("Received packet")
        match h.type:
            case CRAPMsgType.AUDIO_START:
                print("Audio start")
            case CRAPMsgType.AUDIO_STOP:
                shouldReceive = False
                print("Audio end")
            case CRAPMsgType.AUDIO_DATA:
                f.write(conn.recv(h.size))
    f.close()

conn.close()
sock.close()
