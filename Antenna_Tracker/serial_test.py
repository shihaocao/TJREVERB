#I hope this worksaa
import serial
import time
ser = serial.Serial('/dev/ttyUSB1',115200)
#s = []

while True:
	ctime = time.time()
	#in1 = input("Give a command: ")
	svalue1 = int((ctime+20)%180)
	svalue2 = int((ctime+30)%180)
	
	res = str(svalue1)+" "+str(svalue2)
	
	#res = 'hi'
	b = bytearray()
	b.extend(map(ord,res))
	ser.write(b)
	read_serial=ser.readline()
	#s[0] = str(int(read_serial,16))
	#print s[0]
	read_serial = str(read_serial.strip())[1:]
	print(read_serial)
