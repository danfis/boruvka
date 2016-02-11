#!/usr/bin/env python
##
# Boruvka
# --------
# Copyright (c)2016 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of Boruvka.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##


from __future__ import print_function
import sys
import re

TYPES = {
    'int8'   : 'int8_t',
    'uint8'  : 'uint8_t',
    'int16'  : 'int16_t',
    'uint16' : 'uint16_t',
    'int32'  : 'int32_t',
    'uint32' : 'uint32_t',
    'int64'  : 'int64_t',
    'uint64' : 'uint64_t',
    'char'   : 'char',
    'uchar'  : 'unsigned char',
    'short'  : 'short',
    'ushort' : 'unsigned short',
    'int'    : 'int',
    'uint'   : 'unsigned int',
    'long'   : 'long',
    'ulong'  : 'unsigned long',
    'float'  : 'float',
    'double' : 'double',
}

ZEROS = {
    'int8'   : '(int8_t)0',
    'uint8'  : '(uint8_t)0',
    'int16'  : '(int16_t)0',
    'uint16' : '(uint16_t)0',
    'int32'  : '(int32_t)0',
    'uint32' : '(uint32_t)0',
    'int64'  : '(int64_t)0',
    'uint64' : '(uint64_t)0',
    'char'   : '(char)0',
    'uchar'  : '(unsigned char)0',
    'short'  : '(short)0',
    'ushort' : '(unsigned short)0',
    'int'    : '(int)0',
    'uint'   : '(unsigned int)0',
    'long'   : '(long)0',
    'ulong'  : '(unsigned long)0',
    'float'  : '0.f',
    'double' : '0.',
}

STRUCTS = {}

MAX_MEMBERS = 32
HEADER_TYPE = 'uint32_t'

class Member(object):
    def __init__(self, id, name, type, default, comment):
        self.id = id
        self.name = name
        self.type = None
        self.default = default
        self.comment = comment
        self.is_arr = False
        self.struct = None

        basetype = type
        if type.endswith('[]'):
            self.is_arr = True
            basetype = type[:-2]
            if default is not None:
                print('Error: Arrays cannot have default value.', file = sys.stderr)
                sys.exit(-1)

        if basetype in TYPES:
            self.type = basetype
        elif basetype in STRUCTS:
            self.type = 'struct'
            self.struct = STRUCTS[basetype]
        else:
            print('Error: Unkown type {0}'.format(basetype), file = sys.stderr)
            sys.exit(-1)

    def genCStructMember(self, fout):
        asterix = ''
        if self.is_arr:
            asterix = '*'

        line = '    {0} {1}{2};{3}\n'
        comm = ''
        if self.comment is not None:
            comm = self.comment
        if self.type == 'struct':
            line = line.format(self.struct.name, asterix, self.name, comm)
        else:
            line = line.format(TYPES[self.type], asterix, self.name, comm)
        fout.write(line)

        if self.is_arr:
            fout.write('    int {0}_size;\n'.format(self.name))
            fout.write('    int {0}_alloc;\n'.format(self.name))

    def cDefaultVal(self):
        if self.default is not None:
            return self.default

        if self.is_arr:
            return 'NULL, 0, 0'

        if self.type == 'struct':
            return self.struct.cDefaultVal()


        return ZEROS[self.type]

    def cSchema(self, struct_name):
        foffset = '_BOR_MSG_SCHEMA_OFFSET({0}, {{0}})'.format(struct_name)

        stype = '_BOR_MSG_SCHEMA_' + self.type.upper()
        soffset = foffset.format(self.name)
        ssize_offset = '-1'
        salloc_offset = '-1'
        ssub = 'NULL'
        sdefault = '(void *)(((char *)&___{0}_default) + {1})'.format(struct_name, soffset)

        if self.type == 'struct':
            stype = '_BOR_MSG_SCHEMA_MSG'
            ssub = '&___{0}_schema'.format(self.struct.name)
            sdefault = 'NULL'

        if self.is_arr:
            stype = '_BOR_MSG_SCHEMA_ARR_BASE + ' + stype
            ssize_offset = foffset.format('{0}_size'.format(self.name))
            salloc_offset = foffset.format('{0}_alloc'.format(self.name))
            sdefault = 'NULL'


        sline = '{{{0}, {1}, {2}, {3}, {4}, {5}}}'.format(stype, soffset,
                                                     ssize_offset,
                                                     salloc_offset, ssub,
                                                     sdefault)
        return sline

    def cHeaderMacro(self, idx, struct_name):
        s = '#define BOR_MSG_HEADER_{0}_{1} {2}'
        s = s.format(struct_name, self.name, idx)
        return s

class Struct(object):
    def __init__(self, name):
        self.name = name
        self.members = []
        self.before = ''
        self.after = ''

        STRUCTS[self.name] = self

    def addMember(self, name, type, default = None, comment = None):
        if len(self.members) == MAX_MEMBERS:
            print('Error: Exceeded maximal number of struct members ({0})!'
                    .format(MAX_MEMBERS), file = sys.stderr)
            sys.exit(-1)
        m = Member(len(self.members), name, type, default, comment)
        self.members += [m]

    def finalize(self, before):
        self.before = before

    def addAfter(self, after):
        self.after = after

    def genCStruct(self, fout):
        fout.write(self.before)
        fout.write('struct _{0} {{\n'.format(self.name))
        fout.write('    {0} __msg_header;\n'.format(HEADER_TYPE))
        for m in self.members:
            m.genCStructMember(fout)
        fout.write('};\n')
        fout.write('typedef struct _{0} {0};\n'.format(self.name))
        fout.write('extern bor_msg_schema_t *{0}_schema;\n'.format(self.name))
        fout.write(self.after)

    def cDefaultVal(self):
        val = [m.cDefaultVal() for m in self.members]
        val = '{ 0, ' + ', '.join(val) + ' }'
        return val

    def genCDefault(self, fout):
        default = [m.cDefaultVal() for m in self.members]
        fout.write('static {0} ___{0}_default = '.format(self.name))
        fout.write(self.cDefaultVal())
        fout.write(';\n')

    def genCSchema(self, fout):
        fields = [m.cSchema(self.name) for m in self.members]
        fields = ['    ' + x for x in fields]
        fields = ',\n'.join(fields)

        fout.write('static bor_msg_schema_field_t ___{0}_fields[] = {{\n'.format(self.name))
        fout.write(fields + '\n')
        fout.write('};\n');
        fout.write('static bor_msg_schema_t ___{0}_schema = {{\n'.format(self.name))
        fout.write('    0,\n')
        fout.write('    sizeof({0}),\n'.format(self.name))
        fout.write('    ___{0}_fields,\n'.format(self.name))
        fout.write('    sizeof(___{0}_fields) / sizeof(bor_msg_schema_field_t),\n'.format(self.name))
        fout.write('    &___{0}_default\n'.format(self.name))
        fout.write('};\n');
        fout.write('bor_msg_schema_t *{0}_schema = &___{0}_schema;\n'.format(self.name))
        fout.write('\n')

    def genCHeaderMacros(self, fout):
        f = [m.cHeaderMacro(i, self.name) for i, m in enumerate(self.members)]
        fout.write('\n'.join(f))
        fout.write('\n\n')

def parseStructs():
    structs = []
    s = None
    comment_line = None
    before = ''
    after = ''
    linebuf = ''
    for i, line in enumerate(sys.stdin):
        if s is not None:
            idx = line.find(';')
            if idx >= 0:
                comment_line = line[idx+1:].strip('\n')
                line = line[:idx]
            line = line.strip()
            sline = line.split()
        else:
            sline = line.strip().split()

        if len(sline) == 3 and sline[0] == 'msg' and sline[-1] == '{':
            s = Struct(sline[1])
            structs += [s]
            before = linebuf
            linebuf = ''

        elif s is not None and len(sline) >= 2:
            if len(sline) == 3:
                sline[2] = sline[2].strip('}{')
                s.addMember(sline[0], sline[1], sline[2], comment_line)
            else:
                s.addMember(sline[0], sline[1], comment = comment_line)
            comment_line = None

        elif s is not None and len(sline) == 1 and sline[0] == '}':
            s.finalize(before)
            s = None

        elif s is not None and len(sline) == 0:
            continue

        elif s is not None:
            print('Error: Invalid input line {0}: {1}'.format(i + 1, line),
                  file = sys.stderr)
            sys.exit(-1)

        else:
            linebuf += line

    structs[-1].addAfter(linebuf)

    return structs

def genCH(structs, fout):
    for s in structs:
        s.genCStruct(fout)
        s.genCHeaderMacros(fout)

def genCC(structs, fout):
    for s in structs:
        s.genCDefault(fout)

    fout.write('\n')
    for s in structs:
        s.genCSchema(fout)

if __name__ == '__main__':
    opts = ['--h', '--c']
    if len(sys.argv) != 2 or sys.argv[1] not in opts:
        print('Usage: {0} [--h|--c] <in.sch'.format(sys.argv[0]))
        sys.exit(-1)

    structs = parseStructs()
    if sys.argv[1] == '--h':
        genCH(structs, sys.stdout)
    if sys.argv[1] == '--c':
        genCC(structs, sys.stdout)
