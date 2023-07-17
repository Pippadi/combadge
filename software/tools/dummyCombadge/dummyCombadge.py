#!/usr/bin/python3

import sys
import socket
import time
import threading

PORT = 1592
BUF_LEN = 2048
BYTES_PER_SAMPLE = 2
SAMPLE_RATE = 44100

if len(sys.argv) < 4:
    print("USAGE: dummyCombadge.py <Raw Sound File To Send> <Raw Sound File To Receive> <Remote Combadge>")
    exit(1)

soundFileOut = sys.argv[1]
soundFileIn = sys.argv[2]
remoteBadge = sys.argv[3]

def sendData():
    outgoing = []
    with open(soundFileOut, "rb") as f:
        outgoing = f.read()

    outgoingConn = socket.socket()
    outgoingConn.connect((remoteBadge, PORT))
    time.sleep(0.25)

    i = 0
    for i in range(len(outgoing) // (BUF_LEN * BYTES_PER_SAMPLE)):
        start = i * BYTES_PER_SAMPLE * BUF_LEN
        end = (i + 1) * BYTES_PER_SAMPLE * BUF_LEN
        outgoingConn.sendall(bytes(outgoing[start:end]))
        time.sleep(BUF_LEN/2/SAMPLE_RATE)

    outgoingConn.sendall(bytes(outgoing[i * BYTES_PER_SAMPLE * BUF_LEN:]))
    outgoingConn.close()

def recvData():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("0.0.0.0", PORT))

    f = open(soundFileOut, "wb")
    sock.listen()
    try:
        client, addr = sock.accept()
        print(addr)
        while True:
            incoming = client.recv(BUF_LEN * BYTES_PER_SAMPLE)
            if not incoming:
                break
            else:
                f.write(incoming)
    except:
        pass

    f.close()
    sock.close()

sendThread = threading.Thread(target=sendData)
recvThread = threading.Thread(target=recvData)
sendThread.start()
recvThread.start()
sendThread.join()
recvThread.join()
