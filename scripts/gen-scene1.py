#!/usr/bin/env python

import sys
import os

try:
    density = float(sys.argv[1])
    height = float(sys.argv[2])
    width = float(sys.argv[3])
except:
    print('Usage: {0} density height width'.format(sys.argv[0]))


os.system('./gen-square.py {0} 0 10 0 10'.format(int(density * 10 * 10)))
os.system('./gen-square.py {0} 10 23 0 3'.format(int(density * 13 * 3)))
os.system('./gen-square.py {0} 23 33 0 10'.format(int(density * 10 * 10)))
os.system('./gen-square.py {0} 13 20 {1} 10'.format(int(density * 7 * (7 - height)), 3. + height))
os.system('./gen-square.py {0} {1} {2} 3 {3}'.format(int(density * height * width), \
                                                     16.5 - width / 2, \
                                                     16.5 + width / 2, \
                                                     3 + height))
