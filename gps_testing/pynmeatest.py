import pynmea2

raw = "$GPGGA,184353.07,1929.045,S,02410.506,E,1,04,2.6,100.00,M,-33.9,M,,0000*6D"
#raw2 = ''
raw2 = '$GPGGA,211348.00,3849.0768,N,07710.0925,W,1,06,1.7,75.28,M,-33.40,M,,*62'
#lll = [raw]
raw3 = '$GPGGA,211519.00,,,,,0,00,9.9,,,,,,*69'
lll = [raw,raw2,raw3]

#lll = [x for x in lll if x]

for x in lll:
	msg = pynmea2.parse(x)
	print(msg)
	print("msg.lon: "+str(msg.lon))
	print(msg.lon)
