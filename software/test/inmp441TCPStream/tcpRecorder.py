import socket

BUF_LEN = 512
BYTES_PER_SAMPLE = 2
PORT = 1592

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# sock.bind((socket.gethostbyname(socket.gethostname()), PORT))
sock.bind(("0.0.0.0", PORT))

f = open("recorded.raw", "wb")
sock.listen()
while True:
    try:
        client, addr = sock.accept()
        print(addr)
        while True:
            data = client.recv(BUF_LEN * BYTES_PER_SAMPLE)
            if not data:
                break
            else:
                print("Count of bytes received:", len(data))
                f.write(data)
    except:
        break

f.close()
sock.close()
