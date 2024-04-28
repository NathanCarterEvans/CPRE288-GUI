import socket
import time
import json

HOST = '127.0.0.1'
PORT = 288

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(5)
print("Server is listening on {}:{}".format(HOST, PORT))

their_obj = {
        "distance": 30.0, #
        "angle_middle": 60, # middle of obstacle
        "size": 40.3, # arclength
        }

their_scan = {
        "angle": 100,
        "distance": 100, #cm?
        }

angle = 0
while True:
    client_socket, client_address = s.accept()
    print("Connection from:", client_address)

    with client_socket:
        while True:
            angle = (angle + 5) % 180
            their_scan["angle"] = angle
            try:
                if angle == 0:
                    obstacle_data = json.dumps(their_obj)
                    client_socket.sendall(obstacle_data.encode())
                scan_data = json.dumps(their_scan)
                client_socket.sendall(scan_data.encode())
                time.sleep(0.5)
                # r = client_socket.recv(30)
                # print(r.decode())
            except BrokenPipeError:
                print('connection closed')
                break
