import sys
import serial
import time

debug = True

global ser

def sendCommand(cmd):
    if cmd[-1] != '\r\n':
        cmd += '\r\n'
    if debug:
        print("Sending command: {}".format(cmd))
    ser.write(cmd.encode('UTF-8'))
    ser.flush()
    cmd_echo = ser.readline()
    if debug:
        print("Echoed: " + cmd_echo.decode('UTF-8'))
def setup(port):
    global ser
    ser = serial.Serial(port=port, baudrate = 19200, timeout = 15)
    ser.flush()
    doTheOK()

def doTheOK():
    sendCommand("AT")
    ser.readline().decode('UTF-8') # get the empty line
    resp = ser.readline().decode('UTF-8')
    print (resp)
    if 'OK' not in resp:
        print("Echo"+resp)
        exit(-1)

    # show signal quality
    sendCommand('AT+CSQ')
    ser.readline().decode('UTF-8') # get the empty line
    resp = ser.readline().decode('UTF-8')
    ser.readline().decode('UTF-8') # get the empty line
    ok = ser.readline().decode('UTF-8') # get the 'OK'
    # print("resp: {}".format(repr(resp)))
    if 'OK' not in ok:
        print('Unexpected "OK" response: ' + ok)
        exit(-1)
    sendCommand("AT+SBDMTA=0")
    if debug:
        print("Signal quality 0-5: " + resp)

def main():
    argument = " "
    command = " "
    global port
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        port = '/dev/ttyUSB0'
    setup(port)
    if len(sys.argv) < 4:
        print("not enought args")
        exit(-1)
    else:
        if sys.argv[2] == "message":
            command = sys.argv[3]
            print("Message to send: "+command)
        elif sys.argv[2] == "command":
            argument = sys.argv[3]
            print("Command to execute: "+argument)
        elif sys.argv[2] == "listen":
            print("Listening for Ring")
            listenUp()
        else:
            print("argument 3 is not valid, say either command, message or listen")
            exit(-1)

    #setup(port)
    if debug:
        print("Connected to {}".format(ser.name))

    # clear everything in buffer
    #ser.reset_input_buffer()
    #ser.reset_output_buffer()
    # disable echo
    # sendCommand('ATE0', has_resp=True)

    
    if ' ' not in argument:
        print("Sending command: "+argument)
        sendCommand(argument)
        exit(-1)
    if ' ' not in command:
        print('Sending Message: '+command)
        send(command)
def listenUp():
    ser = serial.Serial(port=port, baudrate = 19200, timeout = 1)
    sendCommand("ST+SBDMTA=1")
    signalStrength = 0
    ringSetup = 0
    iteration = 0
    while ringSetup != 2 :
        ring = ser.readline().decode('UTF-8')
        print(ring)
        if "SBDRING" in ring:
            bytesLeft=1
            ser.timeout=none
            while bytesLeft != 0:
                sendCommand("AT+SBDIXA")
                resp = serial.readline().decode('UTF-8').split(': ')[1].split(', ')
                while len(resp) <= 2 and len(resp) > 0:
                    resp = serial.readline.decode('UTF-8').split(': ')[1](', ')
                bytesLeft=resp[0]
            sendCommand("AT+SBDRT")
            print(ser.readline().decode('UTF-8'))
            ringSetup = 0
            sendCommand("at+sbdmta=0")
            break
        #ser.flush()
        print("listening...")

def send(thingToSend):
    # try to send until it sends
    startTime = time.time()
    alert = 2
    while alert == 2:
        # prepare message
        sendCommand("AT+SBDWT=" + thingToSend)
        ok = ser.readline().decode('UTF-8') # get the 'OK'
        ser.readline().decode('UTF-8') # get the empty line

        # send message
        sendCommand("AT+SBDI")
        resp = ser.readline().decode('UTF-8') # get the empty line
        resp = resp.replace(",", " ").split(" ")
        startTime = time.time()
        currTime = startTime
        while len(resp) > 0 and len(resp) <= 2:
            print(resp)
            resp = ser.readline().decode('UTF-8')    
            resp = resp.replace(",", " ").split(" ")
            curTime = time.time()
            if (curTime-startTime)>30:
                print("time out moving on")
                break
        # get the rsp
        
          #  if debug:
        #print("resp: {}"t )
        try:
            print("*******************" + str(resp))
            alert = int(resp[1])
            print(alert)
        except:
            print("********************exception thrown")
            continue

        #if debug:
            #print("alert: {}".format(alert))
    exit(-1)

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(len(sys.argv))
        print("Usage: $0 <serial port> <command or message> <text to send or command>")
        exit(-1)
    try:
        print(len(sys.argv))
        main()
    finally:
        # sendCommand('ATE1', has_resp=False)
        pass

