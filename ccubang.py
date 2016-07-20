import sys,time,os, re
from datetime import date
from time import sleep
sockdir="/home/fnaltest/TriDAS/pixel/BPixelTools/tools/python"
if not sockdir in sys.path: sys.path.append(sockdir)
from SimpleSocket import SimpleSocket

########################################################

# configuration
ccu_addr = '0x7b'
channel = '0x10'
fechost = 'localhost'      
fecport =  2020

print "connecting to ccu,",fechost, fecport
ccu=SimpleSocket( fechost, fecport)
print " done"
def send(x):
    return ccu.send(x).readlines()
def i2cr(v):
    x = send('i2cr %s %x' % (channel, v))
    x = x[-2]
    assert x.startswith('---->  Value: 0x')
    x = int(x.split()[-1], 16)
    return x
def i2c(a,v):
    x = send('i2c %s %x %x' % (channel, a, v))
    print x
    x = x[-2]
    assert x.startswith('-->CR: Changed Value from ')
    x = x.split()
    old, new = x[-3], x[-1]
    assert old.startswith('0x') and new.startswith('0x')
    assert int(new, 16) == v
    return int(old, 16)

send('ccu ' + ccu_addr)
send('channel ' + channel)

for line in open('the_portal'):
    #print 'LINE', line
    line = line.strip()
    line = line.split()
    assert len(line) == 2
    a = int(line[0], 16)
    b = int(line[1], 16)
    print 'i2c ccu %s channel %s addr %x val %x' % (ccu_addr, channel, a, b)
    i2c(a,b)
