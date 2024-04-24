import tkinter as tk
import socket

# Function to send command to CyBot
def send_command(entry, text_output):
    command = entry.get()
    entry.delete(0, tk.END)  # Clear the entry field after getting the text

    # No need to start a thread for receiving messages since we are not handling incoming messages in this example
    # If you wish to handle responses from CyBot, you will have to implement that part.

    try:
        # Connect to the CyBot
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(3)  # Set timeout for the connection attempt
        client_socket.connect((host, port))
        
        # Send command
        client_socket.sendall(command.encode())
        text_output.insert(tk.END, f"Sent: {command}\n")
        client_socket.close()

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

