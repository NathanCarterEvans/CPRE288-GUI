import tkinter as tk
import math
import socket
import threading

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
cybot_size = 10  # Size of the CyBot's representation
canvas.create_oval(center_x-cybot_size, center_y-cybot_size, center_x+cybot_size, center_y+cybot_size, fill='red')

def plot_point(distance, angle):
    angle_rad = math.radians(angle)
    x = center_x + distance * math.cos(angle_rad)
    y = center_y - distance * math.sin(angle_rad)
    canvas.create_oval(x-5, y-5, x+5, y+5, fill='blue')

client_socket = None

# Function to handle incoming messages from the CyBot
def receive_messages():
    global client_socket
    while True:
        try:
            message = client_socket.recv(1024).decode()
            if message:
                distance, angle = map(int, message.split(','))
                plot_point(distance, angle)
            else:
                break
        except OSError:
            break
        except Exception as e:
            print(f"Error: {e}")
            break

def connect_to_cybot():
    global client_socket
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(3)
        client_socket.connect(('192.168.1.1', 288))
        threading.Thread(target=receive_messages, daemon=True).start()
    except Exception as e:
        print("Could not connect to CyBot:", e)

def send_command(command):
    global client_socket
    try:
        client_socket.sendall(command.encode())
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
button_scan = tk.Button(root, text="Scan", command=lambda: send_command('q'))
button_scan.pack(pady=10)

root.mainloop()

