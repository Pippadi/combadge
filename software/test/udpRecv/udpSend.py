import socket
import time

BUF_LEN = 256

COMBADGE_IP = "192.168.142.93"
PORT = 1592

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    sock.sendto(b'A' * BUF_LEN, (COMBADGE_IP, PORT))
    time.sleep(1)
