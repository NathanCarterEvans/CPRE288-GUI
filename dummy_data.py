import socket
import time

HOST = '127.0.0.1'
PORT = 288

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(5)
print("Server is listening on {}:{}".format(HOST, PORT))

# pub angle: i32, // 90 degrees is forward, 180 is left, 0 is right
# pub sound: f32, // cm
# pub ir: i32,
angle = 0
while True:
    client_socket, client_address = s.accept()
    print("Connection from:", client_address)

    with client_socket:
        while True:
            angle = (angle + 5) % 180
            data = f'{{ "angle": {angle}, "sound": 98.4, "ir": 0 }}'
            try:
                client_socket.sendall(data.encode())
                time.sleep(0.5)
            except BrokenPipeError:
                print('connection closed')
                break
