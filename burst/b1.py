import serial
import time
#from time import *
import sys
#run python b1.py <bperiod>
bp = float(sys.argv[1])
ccc = sys.argv[2]
target = int(sys.argv[3])

print(bp)
print(ccc)
print(target)



state = True
#if bp == 0; don't beacon
if bp == 0:
    bp = 999999
    state = False
ser = serial.Serial(ccc,19200)  # open serial port
print(ser.name)         # check which port was really used
#ser.write(b'hello\n')     # write a string
#ser.close()
oldtime = time.time()
counter = 0
time.sleep(2)
while(counter<50):
    rawtime = time.time()

    #bytes_waiting = sp_input_waiting(port);

    #RISHABH THIS IS HOW YOU READ DATA LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOK
    if (ser.in_waiting > 0):
        rb = ser.read_until()
        print(str(rb))

        #ser.write(b'python response\n')
    #END READ DATA
    if(rawtime - oldtime > bp) and state:
        counter += 1
        #printf("SENDING SAT PERMA BEACON\n");
        #write_array(("SAT PERMA BEACON \n"));
        #msg = 'sat_py_beacon_'+str(counter)
        temp = bytes('sat_py_beacon_'+'bp_'+str(bp)+'-'+str(counter)+'_bytes='+str(target)+':', encoding='utf-8')
        #(len(temp))
        temp += bytes('a'*(target-len(temp)-4)+'END'+'\n', encoding='utf-8')
        print('sent message: '+str(len(temp))+'::'+str(counter))
        print(temp)
        ser.write(temp)
        #ser.write(b'SAT PYTHON BEACON\n')f
        oldtime = time.time()
ccc = 0
while(ccc<50):
    ccc+=1
    time.sleep(2)
    #ser.write(bytes('end', encoding='utf-8'))
    print('END 100')
