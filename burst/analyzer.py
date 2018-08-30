import sys


filename = str(sys.argv[1])
lines = open(filename,'r').readlines()

corrupts = 0
shorts = 0
losts = 0

counter = 0
goalbytes = -1
bperiod = -1
datafails = 0
for line in lines:

    if '_' in line:
        counter += 1
        if 'END' not in line:
            shorts += 1
            continue
        if line[-1] == '.':
            line = line[:-1]
        if 'bp_' in line and '-' in line:
            bt = line[line.index('bp_')+3:line.index('-')]
            if bt is not bperiod:
                print('datafail')
                bperiod = bt
                datafails += 1
        else:
            corrupts += 1
            continue
        if 'bytes_' in line and ':' in line:
            gt = line[line.index('bytes_')+1:line.index(':')]
            if gt is not goalbytes:
                goalbytes = gt
                print('goalbytesfail')
                datafails += 1
        else:
            corrupts += 1
            continue
        if '_bytes' in line:
            messagenum = line[line.index('-')+1:line.index('_bytes')]
            if messagenum>counter:
                losts += messagenum-counter
        else:
            corrupts += 1
            continue
print('DATAFAILS: '+str(datafails))
print('GOALBYTES: '+str(goalbytes))
print('BPERIOD: '+str(bperiod))
print('----------')
print('CORRUPTS: '+str(corrupts))
print('SHORTS: '+str(shorts))
print('LOSTS: '+str(losts))
