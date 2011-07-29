#!/usr/bin/env python

import sys
import re

re_ws = re.compile(r'(^\s+)|(\s+$)')
re_comment = re.compile(r'(//.*)|(/\*.*\*/)')
re_op = re.compile(r'\s*(,|\+|-|\*|%|/|<|>|==|=|>=|<=|;|\||&|>>|<<|:|\?)\s*')
re_comment1 = re.compile('^\s*/\*.*$')
re_comment2 = re.compile('^\s*\*+/.*$')


if len(sys.argv) != 2:
    print('Usage: {0} var_name <in.cl >out.c'.format(sys.argv[0]))
    sys.exit(-1)

var_name = sys.argv[1]
num_lines = 0

def processSource():
    last_len = 99999999999
    in_comment = False
    lines = []
    for line in sys.stdin:
        if in_comment:
            m = re_comment2.match(line)
            if m is not None:
                in_comment = False
            continue

        m = re_comment1.match(line)
        if m is not None:
            in_comment = True
            continue

        line = line.replace('"', '\\"')
        line = re_ws.sub('', line)
        line = re_comment.sub('', line)
        line = re_op.sub(r'\1', line)

        if len(line) > 0 and line[0] == '#':
            line = line + '\\n'

        if len(line) > 0:
            if last_len + len(line) < 80:
                lines[-1] += line
            else:
                lines.append(line)
            last_len = len(lines[-1])

    return lines

lines = processSource()

print('static char *__{0}[] = {{'.format(var_name))

for line in lines:
    print('    "{0}",'.format(line))
    num_lines += 1
    
print('};')
print('static const char **{0} = (const char **)__{0};'.format(var_name))
print('static size_t {0}_len = {1};'.format(var_name, num_lines))
#print('static size_t {0}_len = {1};'.format(var_name, num_lines))
