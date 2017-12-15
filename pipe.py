from __future__ import print_function
import time
import struct
import re
import sys

f = open(r'/tmp/myfifo', 'r+b', 0)
i = 1

while True:
    s = 'Message[{0}]'.format(i)
    i += 1


    n = struct.unpack('I', f.read(4))[0]    # Read str length
    s = f.read(n)                           # Read str
    if re.search('[a-zA-Z0-9]', s):
    #f.seek(0)                               # Important!!!
        print (s, end='')
        sys.stdout.flush()


