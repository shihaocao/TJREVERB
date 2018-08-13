import pynmea2

raw = "$GPGGA,184353.07,1929.045,S,02410.506,E,1,04,2.6,100.00,M,-33.9,M,,0000*6D"
#raw2 = ''
lll = [raw]
#lll = [raw,raw2]
#lll = [x for x in lll if x]

for x in lll:
	msg = pynmea2.parse(x)
	print(msg)
	print("msg.lon: "+str(msg.lon))
