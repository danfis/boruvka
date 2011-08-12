#!/usr/bin/env python

import sys
import re
import os

include_fermat_pat = re.compile('^#\w*include <fermat/(.*)>$')
include1_pat = re.compile('^#\w*include <(.*)>$')
include2_pat = re.compile('^#\w*include "(.*)"$')

class File(object):
    def __init__(self, fn):
        self.fn  = fn
        self.fn2 = os.path.split(fn)[1]
        self.fermat_deps = []
        self.deps1 = []
        self.deps2 = []
        self._findDeps()

    def _findDeps(self):
        fin = open(self.fn, 'r')
        if not fin:
            return

        for line in fin:
            match = include1_pat.match(line)
            if match:
                f = match.group(1)
                if f.startswith('fermat/'):
                    self.fermat_deps.append(f)
                else:
                    self.deps1.append(f)

            match = include2_pat.match(line)
            if match:
                f = match.group(1)
                if f.startswith('fermat/'):
                    self.fermat_deps.append(f)
                else:
                    self.deps2.append(f)
        fin.close()

    def __str__(self):
        s = '<File({0}): {1}, {2}, {3}'.format(self.fn, str(self.fermat_deps), str(self.deps1), str(self.deps2))
        return s

def findDepsRecursive(fns, files_set = None):
    if not files_set:
        files_set = set()

    files = []
    for fn in fns:
        if not os.path.isfile(fn):
            continue

        if fn not in files_set:
            files_set.add(fn)

            f = File(fn)
            files.append(f)
            files += findDepsRecursive(f.fermat_deps, files_set)

    return files
            

def makedot(fns):
    files = findDepsRecursive(fns)

    fs = set()
    print('digraph G {')
    print('  ratio=auto;')
    print('  concentrate=yes')
    for f in files:
        fs.add(f.fn)
        for f2 in f.fermat_deps:
            fs.add(f2)

    for f in fs:
        print('    "{0}" [ label = "{0}", shape = "box" ];'.format(f))

    for f in files:
        for f2 in f.fermat_deps:
            print('    "{0}" -> "{1}";'.format(f.fn, f2))
    print('}')

def main():
    dot = False

    fns = []
    for fn in sys.argv[1:]:
        if fn == '--dot':
            dot = True
        elif os.path.isfile(fn):
            fns.append(fn)

    if dot:
        makedot(fns)

if __name__ == '__main__':
    main()
