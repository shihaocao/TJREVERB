import sys
import serial
import pyiridium9602 as iridium

iridium_port = iridium.IridiumCommunicator(sys.argv[1])
