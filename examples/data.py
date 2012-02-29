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

data = []

reader = csv.reader(open(sys.argv[1], 'r'), delimiter = ',')
for row in reader:
    row[0] = mapping[row[0]]
    row[1:] = [float(x) for x in row[1:]]
    data.append(row)

#    for r in row[1:]:
#        print(" {0}".format(r), end = "")
#    print()

rng = []
for i in range(0, len(data[0])):
    rng.append([min([x[i] for x in data]), max([x[i] for x in data])])
    if (rng[-1][1] - rng[-1][0]) == 0.:
        rng[-1].append(0)
    else:
        rng[-1].append((RNG[1] - RNG[0]) / (rng[-1][1] - rng[-1][0]))

for row in data:
    print(row[0], end = '')
    for i in range(1, len(row)):
        print(' {0}'.format((row[i] - rng[i][0]) * rng[i][2] + RNG[0]), end = '')
    print()
