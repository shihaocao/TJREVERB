import serial
import time
import sys

from threading import Thread

serialPort = sys.argv[1]
ser = serial.Serial(serialPort, 19200) #open port
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
            print(rr)
            #return rr
def keyin():
    while(True):
        in1 = input("Type command: ")
        send(in1)

def startup():
    t1 = Thread(target=listen, args=())
    t1.daemon = True
    t1.start()

if __name__ == '__main__':
    startup()
    t2 = Thread(target=keyin, args=())
    t2.daemon = True
    t2.start()
    while True:
        time.sleep(1)
