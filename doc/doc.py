#!/usr/bin/env python

import sys, re

pat_comment_start = re.compile(r'^/\*\*$')
pat_comment_end   = re.compile(r'^ \*/$')
pat_func_start    = re.compile(r'^[a-z0-9_][a-z0-9_ ]* \**[a-zA-Z0-9]+\(.*$')
pat_typedef       = re.compile(r'^typedef .* ([a-z0-9_]+_t);$')
pat_struct_start  = re.compile(r'(struct|union) ([a-z_0-9]+_t).*{$')
pat_struct_end    = re.compile(r'^}.*;$')
pat_macro         = re.compile(r'^#define ([A-Z_]+\([a-zA-Z_, ]*\) ).*$')
pat_macro2        = re.compile(r'^#define ([A-Z_]+ ).*$')
pat_macro_inline  = re.compile(r'^# *define ([A-Z_]+\([a-zA-Z_, ]*\)) .*/\*!< (.*) \*/$')
pat_macro_inline2 = re.compile(r'^# *define ([A-Z0-9_]+) .*/\*!< (.*) \*/$')
pat_code_block_start = re.compile(r'^/\*\* v+ \*/$')
pat_code_block_end   = re.compile(r'^/\*\* \^+ \*/$')
pat_inline_code_block_start = re.compile(r'^~+$')
pat_inline_code_block_end   = re.compile(r'^~+$')
pat_see_struct = re.compile(r'^See ([a-z0-9_]+_t).$')
pat_monotype = re.compile(r'\{([a-zA-Z0-9*_-]+\(?\)?)\}')

CONTEXT = None
STRUCTS = {}

class Element(object):
    def _formatCommentLine(self, line, line_prefix):
        s = line_prefix + line + '\n'

        if len(line) == 0:
            s = '\n'
        else:
            s = pat_monotype.sub(r'``\1``', s)

            if CONTEXT == 'vec':
                if (line.find('=') > 0 \
                        and (line.find('+') >= 0 \
                            or line.find('-') >= 0 \
                            or line.find('*') >= 0 \
                            or line.find('/') >= 0)) \
                    or (line.startswith('a.x = ') \
                            or line.startswith('a.y = ') \
                            or line.startswith('a.z = ')):
                    
                    if line.find('.x') >= 0 or line.find('.y') >= 0 or line.find('.z') >= 0 or line.find('.w') >= 0:
                        line = line.replace('.x', '_x')
                        line = line.replace('.y', '_y')
                        line = line.replace('.z', '_z')
                        line = line.replace('.w', '_w')
                    else:
                        line = line.replace("d ", "\\vec{d} ")
                        line = line.replace("w", "\\vec{w}")
                        line = line.replace("v ", "\\vec{v} ")
                        line = line.replace(" v", " \\vec{v}")

                    s  = '\n'
                    s += line_prefix + ':math:`'
                    s += line
                    s += '`\n'

        return s

    def _formatCommentCodeBlock(self, line, itline, line_prefix):
        s = ''

        for line in itline:
            if line[:3] == ' * ':
                match = pat_inline_code_block_end.match(line[3:])
                if match:
                    break

                s += self._formatCommentLine(line[3:], line_prefix + '    ')
            elif line == ' *':
                s += line_prefix + '    ' + '\n'

        if len(s) > 0:
            s = '\n' + line_prefix + '.. code-block:: c\n\n' + s + '\n'
        return s

    def _formatSeeStruct(self, key, line_prefix):
        struct = STRUCTS.get(key, None)
        if struct is None:
            return ''

        return struct.formatStruct(line_prefix)

    def formatComment(self, comment, line_prefix = ''):
        itline = iter(comment)
        s = ''
        for line in itline:
            if line == '/**' or line == ' */':
                continue
            if line[:3] == ' * ':
                match = pat_inline_code_block_start.match(line[3:])
                if match:
                    s += self._formatCommentCodeBlock(line, itline, line_prefix)
                    continue

                match = pat_see_struct.match(line[3:])
                if match:
                    s += self._formatSeeStruct(match.group(1), line_prefix)
                    continue

                s += self._formatCommentLine(line[3:], line_prefix)
            elif line == ' *':
                s += line_prefix + '\n'

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

        self.name = 'noname'
        match = pat_typedef.match(typedef)
        if match:
            self.name = match.group(1)

        STRUCTS[self.name] = self

    def formatStruct(self, line_prefix = ''):
        s = ''

        match = pat_typedef.match(self.typedef)
        if match:
            s += line_prefix + '.. c:type:: {0}\n\n'.format(match.group(1))

        s += line_prefix + '.. code-block:: c\n\n'
        for line in self.struct:
            s += line_prefix + '    ' + line + '\n'
        s += line_prefix + '    ' + self.typedef + '\n'

        s += line_prefix + '\n'

        return s

    def format(self):
        s = self.formatComment(self.comment)
        s += '\n'

        s += self.formatStruct()

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

class Macro(Element):
    def __init__(self, comment, macro):
        self.comment = comment
        self.macro   = macro

    def format(self):
        s = '.. c:macro:: {0}\n'.format(self.macro)
        s += '\n'
        s += self.formatComment(self.comment, '    ')
        return s

class MacroInline(Macro):
    def __init__(self, comment_line, macro):
        comment = ['/**\n']
        comment.append(' * ' + comment_line + '\n')
        comment.append('*/\n')
        Macro.__init__(self, comment, macro)

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
        if pat_struct_end.match(line):
            break

    typedef = ''
    line = next(lines_it)
    match = pat_typedef.match(line)
    if match:
        typedef = line

    return Struct(comment, struct, typedef)

def parseMacro(comment, macro):
    return Macro(comment, macro)
def parseMacroInline(comment, macro):
    return MacroInline(comment, macro)

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

    match = pat_macro.match(line)
    if match:
        return parseMacro(comment, match.group(1))

    match = pat_macro2.match(line)
    if match:
        return parseMacro(comment, match.group(1))

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
    global CONTEXT

    if fn.find('vec') >= 0:
        CONTEXT = 'vec'

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

        match = pat_struct_start.match(line)
        if match:
            parseStruct([], line, lines_it)

        match = pat_macro_inline.match(line)
        if match:
            section = parseMacroInline(match.group(2), match.group(1))
            if section is not None:
                sections.append(section)

        match = pat_macro_inline2.match(line)
        if match:
            section = parseMacroInline(match.group(2), match.group(1))
            if section is not None:
                sections.append(section)

    for section in sections:
        print(section.format())

if __name__ == '__main__':
    for fn in sys.argv[1:]:
        parse(fn)
