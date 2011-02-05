#!/usr/bin/env python

import re
import subprocess
import math

NUM_RUNS = 100

def median(l):
    if len(l) == 0:
        return -1

    l = sorted(l)
    if len(l) % 2 == 0:
        return (l[len(l) // 2 - 1] + l[len(l) // 2]) / 2
    else:
        return l[len(l) // 2]

def avg(l):
    t = 0
    for i in l:
        t += i
    return t / len(l)

def var2(times):
    average = avg(times)
    cum = 0
    for t in times:
        cum += (t - average)**2
    return cum / len(times)

def var(times):
    return math.sqrt(var2(times))

def main():
    results_keys = ["add", "sub", "scale", "normalize", "dot", "cross",
                    "len2", "len", "dist2", "dist", "segment_dist",
                    "tri_dist", "eq", "neq" ]

    pat = re.compile(r'(.*): (.*) (.*)')
    devnull = open('/dev/null', 'w')

    results = {}
    for key in results_keys:
        results[key] = []
        for i in range(0, NUM_RUNS):
            print('{0:>15s} {1:05d}/{2:05d}'.format(key, i, NUM_RUNS), end = '\r')
            out = subprocess.check_output(['./bench', key], stderr = devnull)
            out = out.decode('ascii')
            out = out.split('\n')
            for line in out:
                match = pat.match(line)
                if match:
                    s  = float(match.group(2))
                    ns = float(match.group(3))
                    results[key].append(s * 1000000. + ns / 1000.)
    devnull.close()

    print('{0:^15s}: {1:^14s} {2:^14s} {3:^14s} {4:^14s} {5:^14s}'.format('Name', 'Median', 'Avg', 'Min', 'Max', 'Var'))
    for key in results_keys:
        results[key].sort()
        r = results[key]
        print('{0:15s}: {1: 14.1f} {2: 14.1f} {3: 14.1f} {4: 14.1f} {5: 14.1f}'.format(key, median(r), avg(r), min(r), max(r), var(r)))



if __name__ == '__main__':
    main()
