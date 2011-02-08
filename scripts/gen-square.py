#!/usr/bin/env python

import sys
from random import uniform

def usage():
    print('Usage: {0} num x_min x_max y_min y_max'.format(sys.argv[0]))
    sys.exit(-1)


def genPoint(ran):
    x = uniform(ran[0], ran[1])
    y = uniform(ran[2], ran[3])
    print(x, y)

def main():
    if len(sys.argv) != 6:
        usage()

    num = int(sys.argv[1])
    ran = [float(x) for x in sys.argv[2:]]

    for i in range(0, num):
        genPoint(ran)


if __name__ == '__main__':
    main()
