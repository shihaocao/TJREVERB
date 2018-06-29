# -*- coding: utf-8 -*-
"""
Created on Tue Jun 26 13:50:42 2018

@author: rmisr
"""

import serial
import time
import sys
import threading


BUFFSIZE = 512
ser = serial.Serial('COM5',19200)  # open serial port
print(ser.name)         # check which port was really used

rawtime = time.time()
oldtime = rawtime
print("HEARTBEAT MODE")
retmsg = ""
while(True):
    rawtime = time.time()
    bytes_waiting = ser.read_until()
    if(bytes_waiting > 0):
        retmsg += ("I'M ALIVE: " + str(rawtime) + "\n")
        byte_buff = ser.read()
        print(byte_buff)    #Doesn't implement number of bytes read, b/c not needed for Python printing
        print("Sending Response " + retmsg + "\n")
        ser.write(retmsg)
        time.sleep(2)