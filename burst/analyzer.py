filename = str(sys.argv[1])
lines = file.open(filename+'.txt').readlines()

corrupts = 0
shorts = 0
losts = 0

counter = 0
goalbytes = -1
bperiod = -1
datafail = 0
for line in lines:

    if '_' in line:
        counter += 1
        if 'END' not in line:
            shorts += 1
            continue
        if line[-1] == '.':
            line = line[:-1]
        if 'bp_' in line and '-' in line:
            bt = line[index.of('bp_')+1:index.of('-')]
            if bt is not bperiod:
                datafail += 1
        else:
            corrupts += 1
            continue
        if 'bytes_' in line and ':' in line:
            gt = line[index.of('bytes_')+1:index.of(':')]
            if gt is not goalbytes:
                datafail += 1
        else:
            corrupts += 1
            continue
        if '_bytes' in line:
            messagenum = line[index.of('-')+1:index.of('_bytes')]
            if messagenum>counter:
                losts += messagenum-counter
        else:
            corrupts += 1
            continue
print('DATAFAILS: '+str(datafails))
print('GOALBYTES: '+str(goalbytes))
print('BPERIOD: '+str(bperiod
print('----------')
print('CORRUPTS: '+str(corrupts))
print('SHORTS: '+str(shorts))
print('LOSTS: '+str(losts))
