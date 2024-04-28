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
    # Convert angle to radians
    angle_rad = math.radians(angle)
    # Convert polar coordinates to Cartesian coordinates
    x = center_x + distance * math.cos(angle_rad)
    # Invert y because the y-axis in Tkinter canvas goes downward
    y = center_y - distance * math.sin(angle_rad)
    # Plot the point
    canvas.create_oval(x-5, y-5, x+5, y+5, fill='blue')  # Use blue to differentiate from the CyBot

# Function to handle incoming messages from the CyBot
def receive_messages(client_socket):
    while True:
        try:
            message = client_socket.recv(1024).decode()
            if message:
                # Expected format "distance,angle"
                distance, angle = map(int, message.split(','))
                plot_point(distance, angle)
            else:  # No message means the connection was closed
                break
        except OSError:  # Socket closed from this end
            break
        except Exception as e:
            print(f"Error: {e}")
            break

def connect_to_cybot():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(3)  # Set timeout for the connection attempt
        client_socket.connect(('192.168.1.1', 288))  # CyBot's IP address and port
        
        # Start the receiving thread
        threading.Thread(target=receive_messages, args=(client_socket,), daemon=True).start()

    except Exception as e:
        print("Could not connect to CyBot:", e)

# Button to start connection
connect_button = tk.Button(root, text="Connect to CyBot", command=connect_to_cybot)
connect_button.pack(pady=10)

root.mainloop()

