import serial
import time
import sys

import submodules.command_ingest as ci

from threading import Thread
from core import config

#open port
def send(msg):
    msg += "\n"
    ser.write(bytes(msg,encoding="utf-8"))
def listen():
    while(True):
        zz = ser.inWaiting()
        rr = b''
        if zz > 0:
            time.sleep(.5)
            rr += ser.read(size = zz)
            ci.dispatch_command(rr)
            #return (rr)
            #return rr


def keyin():
    while(True):
        in1 = input("Type command: ")
        send(in1)


def beacon():
    while(True):
        time.sleep(bperiod)
        btext = "HELLO WORLD I AM FROMETH TJ"
        send(btext)


def on_startup():
    global bperiod, t1, ser
    bperiod = 60
    serialPort = config['aprs']['serial_port']
    ser = serial.Serial(serialPort, 19200)
    t1 = Thread(target=listen, args=())
    t1.daemon = True
    t1.start()

def enter_normal_mode():
    global bperiod
    bperiod = 60


def enter_low_power_mode():
    global bperiod
    bperiod = 120

def enter_emergency_mode():
    pass


if __name__ == '__main__':
    startup()
    serialPort = sys.argv[1]
    ser = serial.Serial(serialPort, 19200)
    t2 = Thread(target=keyin, args=())
    t2.daemon = True
    t2.start()
    while True:
        time.sleep(1)