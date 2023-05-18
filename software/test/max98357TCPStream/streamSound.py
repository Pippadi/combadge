import sys
import socket
import time

BUDDY_IP = "192.168.142.57"
PORT = 1592
BUF_LEN = 2048
BYTES_PER_SAMPLE = 2
SAMPLE_RATE = 44100

if len(sys.argv) < 2:
    print("USAGE: streamSound.py <Raw Sound File>")
    exit(1)

soundFile = sys.argv[1]

data = []
with open(soundFile, "rb") as f:
    data = f.read()

client = socket.socket()
client.connect((BUDDY_IP, PORT))

time.sleep(0.25)

i = 0
for i in range(len(data) // (BUF_LEN * BYTES_PER_SAMPLE)):
    start = i * BYTES_PER_SAMPLE * BUF_LEN
    end = (i + 1) * BYTES_PER_SAMPLE * BUF_LEN
    client.sendall(bytes(data[start:end]))
    time.sleep(BUF_LEN/2/SAMPLE_RATE)

client.sendall(bytes(data[i * BYTES_PER_SAMPLE * BUF_LEN:]))

client.close()
