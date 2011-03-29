#!/usr/bin/env python

import sys

if len(sys.argv) != 2:
    print('Usage: {0} var_name <in.cl >out.c'.format(sys.argv[0]))
    sys.exit(-1)

var_name = sys.argv[1]
num_lines = 0

print('static char *__{0}[] = {{'.format(var_name))

for line in sys.stdin:
    print('    "{0}",'.format(line.replace('\n', '\\n')))
    num_lines += 1
    
print('};')
print('static const char **{0} = (const char **)__{0};'.format(var_name))
print('static size_t {0}_len = {1};'.format(var_name, num_lines))
#print('static size_t {0}_len = {1};'.format(var_name, num_lines))
