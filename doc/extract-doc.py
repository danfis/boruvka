import re
import sys

pat_comment_start = re.compile(r'^/\*\*$')
pat_comment_end   = re.compile(r'^ \*/$')
pat_func_start    = re.compile(r'^[a-z0-9_][a-z0-9_ ]* \**[a-zA-Z0-9]+\(.*$')
pat_typedef       = re.compile(r'^typedef .* ([a-z0-9_]+_t);$')

funcs    = []
typedefs = []

class Func(object):
    def __init__(self, decl = [], comment = []):
        self.decl    = decl
        self.comment = comment

    def declF(self):
        s = ''
        for d in self.decl:
            stripped = d.strip()
            if len(s) > 0 and s[-1] == ',':
                s += ' '
            s += stripped
        s = s.rstrip('; ')
        return s

    def sphinx(self):
        s  = '.. c:function:: {0}\n'.format(self.declF())
        s += '\n'

        for line in self.comment:
            if line == '/**' or line == ' */':
                continue
            if line[:3] == ' * ':
                s += '    '
                s += line[3:] + '\n'
            elif line == ' *':
                s += '\n'

        return s

class Typedef(object):
    def __init__(self, name = ''):
        self.name = name

    def sphinx(self):
        s = '.. c:type:: {0}\n'.format(self.name)
        return s

def parseFuncs(lines_in):
    global funcs

    comment = []
    decl    = []

    match1 = None
    match2 = None
    match3 = None
    state  = 0

    lines_out = []
    for line in lines_in:
        #print(state, line)
        if state == 2:
            match = pat_func_start.match(line)
            if not match:
                comment = []
                decl    = []
                state = 0
            else:
                state = 3

        if state == 0:
            match = pat_comment_start.match(line)
            if match:
                comment = [line]
                state = 1 # in comment
            else:
                match = pat_func_start.match(line)
                if match:
                    state = 3
                else:
                    lines_out.append(line)

        elif state == 1:
            comment.append(line)

            match = pat_comment_end.match(line)
            if match:
                state = 2 # after comment

        if state == 3:
            decl.append(line)
            if line[-1] == ';':
                funcs.append(Func(decl, comment))
                comment = []
                decl    = []
                state = 0

    return lines_out

def parseTypedefs(lines):
    global typedefs
    names = []

    lines_out = []
    for line in lines:
        match = pat_typedef.match(line)
        if match:
            name = match.group(1)
            if name not in names:
                names.append(name)
        else:
            lines_out.append(line)

    for name in names:
        typedefs.append(Typedef(name))

    return lines_out

def removeInlines(lines):
    lines_out = []
    for line in lines:
        if line.find('INLINES') >= 0:
            return lines_out
        else:
            lines_out.append(line)

def parseLines(lines):
    lines = removeInlines(lines)
    lines = parseFuncs(lines)
    lines = parseTypedefs(lines)

    for t in typedefs:
        print(t.sphinx())
    for f in funcs:
        print(f.sphinx())

def fileLines(fn):
    lines = []
    fin = open(fn, 'r')
    if not fin:
        return None

    for line in fin:
        lines.append(line.rstrip())

    fin.close()

    return lines

def main():
    lines = fileLines(sys.argv[1])
    if lines is None:
        sys.stderr.write("Can't read `{0}'".format(fn))
    else:
        parseLines(lines)

if __name__ == '__main__':
    main()
