#!/usr/bin/env python

import sys, re

pat_comment_start = re.compile(r'^/\*\*$')
pat_comment_end   = re.compile(r'^ \*/$')
pat_func_start    = re.compile(r'^[a-z0-9_][a-z0-9_ ]* \**[a-zA-Z0-9]+\(.*$')
pat_typedef       = re.compile(r'^typedef .* ([a-z0-9_]+_t);$')
pat_struct_start  = re.compile(r'(struct|union) ([a-z_0-9]+_t).*{$')
pat_code_block_start = re.compile(r'^/\*\* v+ \*/$')
pat_code_block_end   = re.compile(r'^/\*\* \^+ \*/$')

class Element(object):
    def formatComment(self, comment, line_prefix = ''):
        s = ''
        for line in comment:
            if line == '/**' or line == ' */':
                continue
            if line[:3] == ' * ':
                s += line_prefix
                s += line[3:] + '\n'
            elif line == ' *':
                s += '\n'

        return s

class Text(Element):
    def __init__(self, comment):
        self.comment = comment

    def format(self):
        return self.formatComment(self.comment)

class Func(Element):
    def __init__(self, comment, decl):
        self.comment = comment
        self.decl    = decl

    def formatFunc(self, decl):
        s = ''
        for d in decl:
            stripped = d.strip()
            if len(s) > 0 and s[-1] == ',':
                s += ' '
            s += stripped
        s = s.rstrip('; ')
        s = '.. c:function:: {0}\n'.format(s)
        return s

    def format(self):
        s  = self.formatFunc(self.decl)
        s += '\n'
        s += self.formatComment(self.comment, '    ')
        return s

class Struct(Element):
    def __init__(self, comment, struct, typedef):
        self.comment = comment
        self.struct  = struct
        self.typedef = typedef

    def format(self):
        s = self.formatComment(self.comment)
        s += '\n'

        match = pat_typedef.match(self.typedef)
        if match:
            s += '.. c:type:: {0}\n\n'.format(match.group(1))

        s += '.. code-block:: c\n\n'
        for line in self.struct:
            s += '    ' + line + '\n'
        s += '    ' + self.typedef + '\n'

        s += '\n'

        return s

class Code(Element):
    def __init__(self, code):
        self.code = code

    def format(self):
        s = '.. code-block:: c\n\n'
        for line in self.code:
            s += '    ' + line + '\n'
        s += '\n'
        return s

def fileLines(fn):
    lines = []
    fin = open(fn, 'r')
    if not fin:
        return None

    for line in fin:
        # omit the rest of file after INLINES mark
        if line.find('INLINES') >= 0:
            break
        lines.append(line.rstrip())

    fin.close()

    return lines

def parseFunc(comment, line, lines_it):
    decl = [line]
    while line[-1] != ';':
        line = next(lines_it)
        decl.append(line)

    return Func(comment, decl)

def parseStruct(comment, line, lines_it):
    struct = [line]
    for line in lines_it:
        struct.append(line)
        if line == '};':
            break

    typedef = ''
    line = next(lines_it)
    match = pat_typedef.match(line)
    if match:
        typedef = line

    return Struct(comment, struct, typedef)

def parseSection(line, lines_it):
    comment = [line]
    for line in lines_it:
        comment.append(line)
        if pat_comment_end.match(line):
            break

    line = next(lines_it)
    if line == '':
        return Text(comment)

    match = pat_func_start.match(line)
    if match:
        return parseFunc(comment, line, lines_it)

    match = pat_struct_start.match(line)
    if match:
        return parseStruct(comment, line, lines_it)
    return None

def parseCode(line, lines_it):
    code = []
    for line in lines_it:
        match = pat_code_block_end.match(line)
        if match:
            break

        code.append(line)

    return Code(code)

def parse(fn):
    lines = fileLines(fn)

    sections = []

    lines_it = iter(lines)
    for line in lines_it:
        match = pat_comment_start.match(line)
        if match:
            section = parseSection(line, lines_it)
            if section is not None:
                sections.append(section)

        match = pat_code_block_start.match(line)
        if match:
            section = parseCode(line, lines_it)
            if section is not None:
                sections.append(section)

    for section in sections:
        print(section.format())

if __name__ == '__main__':
    for fn in sys.argv[1:]:
        parse(fn)
