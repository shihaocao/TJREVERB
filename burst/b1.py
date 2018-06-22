import serial
import time
import sys
#run python b1.py <bperiod>
bp = int(sys.argv[1])
state = True
#if bp == 0; don't beacon
if bp == 0:
    bp = 999999
    state = False
ser = serial.Serial('COM5',19200)  # open serial port
print(ser.name)         # check which port was really used
#ser.write(b'hello\n')     # write a string
#ser.close()
oldtime = time.time()
counter = 0
time.sleep(2)
while(counter<100):
    rawtime = time.time()

    #bytes_waiting = sp_input_waiting(port);

    if (ser.in_waiting > 0):
        rb = ser.read_until()
        print(str(rb))

        ser.write(b'python response\n')


    if(rawtime - oldtime > bp) and state:
        counter += 1
        #printf("SENDING SAT PERMA BEACON\n");
        #write_array(("SAT PERMA BEACON \n"));
        #msg = 'sat_py_beacon_'+str(counter)
        ser.write(bytes('sat_py_beacon_'+'bp_'+str(bp)+'-'+str(counter)+'\n', encoding='utf-8'))
        #ser.write(b'SAT PYTHON BEACON\n')
        oldtime = time.time()

print('END 100')
