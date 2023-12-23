#!/usr/bin/python3

import sys
import socket
import time
import threading
import struct

PORT = 1701
BUF_LEN = 512
BYTES_PER_SAMPLE = 2
SAMPLE_RATE = 44100

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
        return 8 # Two 32-bit ints

if len(sys.argv) < 4:
    print("USAGE: dummyCombadge.py <Raw Sound File To Send> <Raw Sound File To Receive> <Remote Combadge/Bridge>")
    exit(1)

soundFileOut = sys.argv[1]
soundFileIn = sys.argv[2]
remoteBadge = sys.argv[3]

conn = socket.socket()
conn.connect((remoteBadge, PORT))
time.sleep(0.25)

def sendData():
    outgoing = []
    with open(soundFileOut, "rb") as f:
        outgoing = f.read()

    conn.sendall(Header(CRAPMsgType.AUDIO_START, 0).asBytes())

    i = 0
    for i in range(len(outgoing) // (BUF_LEN * BYTES_PER_SAMPLE) + 1):
        start = i * BYTES_PER_SAMPLE * BUF_LEN
        chunk = outgoing[start:start+BUF_LEN*BYTES_PER_SAMPLE]
        conn.sendall(Header(CRAPMsgType.AUDIO_DATA, len(chunk)).asBytes() + bytes(chunk))
        print("Sending packet")
        time.sleep(BUF_LEN/2/SAMPLE_RATE)

    conn.sendall(Header(CRAPMsgType.AUDIO_STOP, 0).asBytes())

def recvData():
    f = open(soundFileIn, "wb")
    shouldReceive = True
    while shouldReceive:
        h = Header()
        h.fromBytes(conn.recv(len(h)))
        match h.type:
            case CRAPMsgType.AUDIO_START:
                print("Audio start")
            case CRAPMsgType.AUDIO_STOP:
                shouldReceive = False
                print("Audio end")
            case CRAPMsgType.AUDIO_DATA:
                f.write(conn.recv(h.size))
    f.close()

sendThread = threading.Thread(target=sendData)
recvThread = threading.Thread(target=recvData)
sendThread.start()
recvThread.start()
sendThread.join()
recvThread.join()

conn.close()
