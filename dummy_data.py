import socket
import time
import json

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
            scan_data = f'{{ "angle": {angle}, "sound": 98.4, "ir": 0 }}'
            try:
                if angle == 0:
                    obs = {
                    "start_detection": 30,
                    "end_detection": 40,
                    "mid_detection": 35,
                    "object_num": 1,
                    "min_distance": 40.4,
                    "radial_width": 10,
                    }
                    obstacle_data = json.dumps(obs)
                    print(f"sending data: {obstacle_data}")
                    client_socket.sendall(obstacle_data.encode())
                client_socket.sendall(scan_data.encode())
                time.sleep(0.5)
                # r = client_socket.recv(30)
                # print(r.decode())
            except BrokenPipeError:
                print('connection closed')
                break
