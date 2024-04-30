import tkinter as tk
import math
import socket
import threading
import json

# Create the main window
root = tk.Tk()
root.title("CyBot Radar")

# Create a canvas for drawing
canvas = tk.Canvas(root, width=400, height=400, bg='white')
canvas.pack()

# Draw the radar
center_x, center_y = 200, 200
radius = 150
canvas.create_oval(center_x-radius, center_y-radius, center_x+radius, center_y+radius, outline='gray')

# Draw the CyBot indicator at the center
cybot_size = 30  # Size of the CyBot's representation
canvas.create_oval(center_x-cybot_size, center_y-cybot_size, center_x+cybot_size, center_y+cybot_size, fill='red')

# List to store scan point references
scan_points = []

def plot_point(distance, angle, fill='blue', radius=5):
    global scan_points
    angle_rad = math.radians(angle)
    x = center_x + distance * math.cos(angle_rad)
    y = center_y - distance * math.sin(angle_rad)
    # Store canvas item reference
    point_id = canvas.create_oval(x-radius, y-radius, x+radius, y+radius, fill=fill)
    scan_points.append(point_id)

def clear_scan():
    global scan_points
    for point in scan_points:
        canvas.delete(point)
    scan_points.clear()  # Clear the list after removing points

def receive_messages():
    global client_socket
    while True:
        try:
            message = client_socket.recv(1024).decode()
            print(f"raw received:\n{message}")
            # Process multiple JSON objects in one message
            messages = [m + '}' for m in message.split('}') if '{' in m]
            for msg in messages:
                print(msg)
                data = json.loads(msg)
                distance = data['distance']
                distance *= 4
                if 'angle' in data:  # the data is a scan
                    angle = data['angle']
                    plot_point(distance, angle)
                if 'size' in data:  # the data is an obstacle
                    size = data['size']
                    middle_angle = data['angle_middle']
                    plot_point(distance, middle_angle, fill='yellow', radius=size)
        except OSError:
            break
        except json.JSONDecodeError:
            print("Error decoding JSON")
        except Exception as e:
            print(f"Error: {e}")
            break

def connect_to_cybot():
    global client_socket
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(10)
        client_socket.connect(('192.168.1.1', 288))
        print("connected!")
        threading.Thread(target=receive_messages, daemon=True).start()
    except Exception as e:
        print("Could not connect to CyBot:", e)

def send_command(command):
    global client_socket
    try:
        client_socket.sendall(command.encode())
        print(f'sent "{command}"')
    except Exception as e:
        print(f"Error sending command {command}: {e}")

# Buttons for robot control
connect_button = tk.Button(root, text="Connect to CyBot", command=connect_to_cybot)
connect_button.pack(pady=10)

button_up = tk.Button(root, text="Up", command=lambda: send_command('w'))
button_up.pack(side='top')
button_down = tk.Button(root, text="Down", command=lambda: send_command('s'))
button_down.pack(side='bottom')
button_left = tk.Button(root, text="Turn Left", command=lambda: send_command('a'))
button_left.pack(side='left')
button_right = tk.Button(root, text="Turn Right", command=lambda: send_command('d'))
button_right.pack(side='right')
button_scan = tk.Button(root, text="Scan", command=lambda: [clear_scan(), send_command('q')])
button_scan.pack(pady=10)
text_input = tk.Text(root, height=1, width=5)
text_input.pack()
send_button = tk.Button(root, text="send cmd", command=lambda: send_command(text_input.get(1.0,2.0).strip()))
send_button.pack()
root.mainloop()

