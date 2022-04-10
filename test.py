from array import array
import socket

sk = socket.socket()

sk.connect(('127.0.0.1', 55555))

for i in range(0, 10000):
    arr = bytearray(256)
    sk.sendall(arr, 256)
    arr2 = sk.recv(256)
sk.close()
