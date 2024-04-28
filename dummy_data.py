import socket
import time

HOST = '127.0.0.1'
PORT = 288

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(5)
print("Server is listening on {}:{}".format(HOST, PORT))

while True:
    client_socket, client_address = s.accept()
    print("Connection from:", client_address)

    with client_socket:
        while True:
            try:
                client_socket.sendall(b"data!!!")
                time.sleep(2)
            except BrokenPipeError:
                print('connection closed')
                break
