import tkinter as tk
import socket
import threading

# This list will store all received messages
received_messages = []

# Function to handle incoming messages from the CyBot
def receive_messages(client_socket, text_output):
    while True:
        try:
            message = client_socket.recv(1024).decode()
            if message:
                received_messages.append(message)  # Store received message
                text_output.insert(tk.END, f"Bot: {message}\n")
            else:  # No message means the connection was closed
                break
        except OSError:  # Socket closed from this end
            break
        except Exception as e:
            text_output.insert(tk.END, f"Error: {e}\n")
            break

# Function to send command to CyBot
def send_command(entry, text_output):
    command = entry.get()
    entry.delete(0, tk.END)  # Clear the entry field after getting the text

    try:
        # Connect to the CyBot
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(3)  # Set timeout for the connection attempt
        client_socket.connect((host, port))
        
        # Start the receiving thread
        threading.Thread(target=receive_messages, args=(client_socket, text_output), daemon=True).start()

        # Send command
        client_socket.sendall(command.encode())
        text_output.insert(tk.END, f"Sent: {command}\n")

    except Exception as e:
        text_output.insert(tk.END, "Command not sent: Could not connect to CyBot.\n")

# Create the main window
root = tk.Tk()
root.title("CyBot Terminal")

# Terminal output
text_output = tk.Text(root, height=10, width=50)
text_output.pack(pady=10, padx=10)

# Entry widget to type commands
entry = tk.Entry(root, width=50)
entry.pack(pady=10, padx=10)
entry.bind("<Return>", lambda event: send_command(entry, text_output))  # Bind the return key to send command

# Send button
send_button = tk.Button(root, text="Send Command", command=lambda: send_command(entry, text_output))
send_button.pack(pady=10)

# CyBot's IP address and port
host = '192.168.1.1'
port = 288

root.mainloop()

