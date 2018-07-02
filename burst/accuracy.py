import time
import serial

NUMREAD = 50 #Number of messages to be read by this program - preset to 100, change accordingly
ser = serial.Serial("/dev/ttyUSB0",19200)

totalRead = 0
correctRead = 0

#print(str(ser.readline()))
#print("IGNORE")
#Based on correct output being: 
# KN4DTQ>SATT4,ARISS:sat_py_beacon_bp_6.0-1_bytes=64:aaaaaaaaaaaaaaaaaaaaaaaaaaaaEND
#with count being any integer from 0-99 (might need to change code in the future to account for larger values of NUMREAD)

while(totalRead < NUMREAD):
    #print("this code works")
    totalRead += 1
    rb = ser.read()
    time.sleep(.5)
    rb += ser.read()
    #rb = str(rb)
    print(str(rb).strip())
    if (len(rb) == 0):
        continue
    elif(rb[:36] != "KN4DTQ>SATT4,ARISS:sat_py_beacon_bp_" or (rb[41:51] != "_bytes=64:" or rb[42:52] != "_bytes=64:") or (rb[80:] != "END" or rb[81:] != "END")):
        continue
    else:
        correctRead += 1
    #totalRead += 1
    #print(str(totalRead)*100)
print("Accuracy: " + str(correctRead*100/totalRead)+ "%") #Format output/printing however you'd like
