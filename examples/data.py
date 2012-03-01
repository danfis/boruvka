#!/usr/bin/env python

import csv
import sys

mapping = {
    'BRICKFACE' : 0,
    'SKY' : 1,
    'FOLIAGE' : 2,
    'CEMENT' : 3,
    'WINDOW' : 4,
    'PATH' : 5,
    'GRASS' : 6
    }

RNG = (-1, 1)

def readData(fn):
    data = []
    reader = csv.reader(open(fn, 'r'), delimiter = ',')
    for row in reader:
        row[0] = mapping[row[0]]
        row[1:] = [float(x) for x in row[1:]]
        data.append(row)
    return data

def compRng(data):
    rng = []
    for i in range(0, len(data[0])):
        rng.append([min([x[i] for x in data]), max([x[i] for x in data])])
        if (rng[-1][1] - rng[-1][0]) == 0.:
            rng[-1].append(0)
        else:
            rng[-1].append((RNG[1] - RNG[0]) / (rng[-1][1] - rng[-1][0]))
    return rng

def scaleData(data, rng):
    for row in data:
        for i in range(1, len(row)):
            row[i] = (row[i] - rng[i][0]) * rng[i][2] + RNG[0]
    return data


train_data = readData(sys.argv[1])
test_data  = readData(sys.argv[2])
data = train_data + test_data

rng = compRng(data)
train_data = scaleData(train_data, rng)
test_data = scaleData(test_data, rng)

def prData(data, rng, name):
    print('{0}_rows:i    = {1}'.format(name, len(data)))
    print('{0}_y:i[{1}] ='.format(name, len(data)))
    for i in range(0, len(data)):
        if i % 10 == 0:
            print('   ', end = '')
        print(' {0}'.format(data[i][0]), end = '')
        if (i + 1) % 10 == 0:
            print()
    print()
    print('{0}_x:f[{1}] ='.format(name, len(data) * (len(data[0]) - 1)))
    for i in range(0, len(data)):
        print('   ', end = '')
        for y in data[i][1:]:
            print(' {0}'.format(y), end = '')
        print()
    print()
    print()


print('classes:i = {1}'.format(name, rng[0][1] + 1))
print('cols:i    = {1}'.format(name, len(data[0]) - 1))
prData(train_data, rng, 'train')
prData(test_data, rng, 'test')
#    for r in row[1:]:
#        print(" {0}".format(r), end = "")
#    print()

#for row in data:
#    print(row[0], end = '')
#    for i in range(1, len(row)):
#        print(' {0}'.format((row[i] - rng[i][0]) * rng[i][2] + RNG[0]), end = '')
#    print()
