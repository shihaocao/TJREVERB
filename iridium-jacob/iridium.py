import pyiridium9602 as iridium
import serial
import sys

iridium_port = iridium.IridiumCommunicator("/dev/ttyUSB1")

def parse_data(data):
    print("My data:", data)

def message_failed(msg_len, content, checksum, calc_check):
    print("Message Failed checksum or length!", msg_len, content, checksum, calc_check)

# Use the default signal class and override the Signal API methods or create your own object.
iridium_port.signal.connected = lambda: print("Connected!")
iridium_port.signal.disconnected = lambda: print("Disconnected!")
iridium_port.signal.serial_number_updated = lambda s: print("Serial Number:", s)
iridium_port.signal.system_time_updated = lambda s: print("System Time:", s)
iridium_port.signal.signal_quality_updated = lambda sig: print("Signal Quality (0-5):", sig)
iridium_port.signal.check_ring_updated = lambda tri, sri: print("Telephone Indicator:", tri, 
                                                                "\nSBD Indicator:", sri)
iridium_port.signal.message_received = parse_data
iridium_port.signal.message_receive_failed = message_failed
iridium_port.signal.notification = print

# NOTE: There is no thread in this example, so `connect()` creates a thread to Complete the connection process
iridium_port.connect() # Raises IridiumError if the port cannot be opened or if the ping did not find a response.

# Non blocking command requests
#iridium_port.request_signal_quality()
#iridium_port.queue_system_time()

# If you run a request immediately after a request then the response will error
# This is because the first command will have it's value returned while the new request is the expected command
#iridium_port.request_serial_number()

# Blocking command (wait for previous command and wait to complete)
with iridium_port.wait_for_command():
    iridium_port.request_signal_quality()

# Blocking command (wait for previous command and wait to complete)
with iridium_port.wait_for_command():
    iridium_port.check_ring()  # If an SBD ring is found automatically start the session to read the value.

# Blocking Command (Do not wait for previous `wait_for_previous=0`)
serial_number = iridium_port.acquire_response(iridium.Command.SERIAL_NUMBER, wait_for_previous=0)
print("Manual Serial Number:", serial_number)

# Pre-made Blocking Command
sig = iridium_port.acquire_signal_quality()
print("Manual Signal Quality (0 - 5):", sig)

iridium_port.send_message("...")

# Stop the `iridium_port.listen_thread` and close the port
iridium_port.close()
