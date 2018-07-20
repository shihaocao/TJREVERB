import sys
import serial

debug = True

def sendCommand(cmd):
    if cmd[-1] != '\r\n':
        cmd += '\r\n'
    if debug:
        print("Sending command: {}".format(cmd))
    ser.write(cmd.encode('UTF-8'))
    ser.flush()
    cmd_echo = ser.readline()
    if debug:
        print("Echoed: {}".format(repr(cmd_echo)))

def main():
    global ser
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        port = '/dev/ttyUSB0'

    ser = serial.Serial(port=port, baudrate=19200, timeout=15)
    
    if debug:
        print("Connected to {}".format(ser.name))

    # clear everything in buffer
    #ser.reset_input_buffer()
    #ser.reset_output_buffer()
    # disable echo
    # sendCommand('ATE0', has_resp=True)

    # make sure its a radio
    sendCommand('AT')
    ser.readline().decode('UTF-8') # get the empty line
    resp = ser.readline().decode('UTF-8')
    print (resp)
    if 'OK' not in resp:
        print("Unexpected response: {}".format(repr(resp)))
        exit(-1)

    # show signal quality
    sendCommand('AT+CSQ')
    ser.readline().decode('UTF-8') # get the empty line
    resp = ser.readline().decode('UTF-8')
    ser.readline().decode('UTF-8') # get the empty line
    ok = ser.readline().decode('UTF-8') # get the 'OK'
    # print("resp: {}".format(repr(resp)))
    if 'OK' not in ok:
        print('Unexpected "OK" response: {}'.format(repr(ok)))
        exit(-1)
    if debug:
        print("Signal quality 0-5: {}".format(repr(resp)))
    
    
    alert = 2

    # try to send until it sends
    while alert == 2:
        # prepare message
        sendCommand("AT+SBDWT=" + sys.argv[2])
        ok = ser.readline().decode('UTF-8') # get the 'OK'
        ser.readline().decode('UTF-8') # get the empty line

        # send message
        sendCommand("AT+SBDI")
        ser.readline().decode('UTF-8') # get the empty line
        resp = ser.readline().decode('UTF-8') # get the rsp
        ser.readline().decode('UTF-8') # get the empty line
        ser.readline().decode('UTF-8') # get the OK
        resp = resp.replace(",", "").split(" ")
        
        if debug:
            print("resp: {}".format(repr(resp)))
        alert = int(resp[1])
        if debug:
            print("alert: {}".format(alert))

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(len(sys.argv))
        print("Usage: $0 <serial port> <text to send>")
        exit(1)
    try:
        print(len(sys.argv))
        main()
    finally:
        # sendCommand('ATE1', has_resp=False)
        pass

