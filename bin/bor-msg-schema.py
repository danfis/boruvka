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

def toCamelCase(name, cap_first_letter = False):
    cap = cap_first_letter
    n = ''
    for l in name:
        if cap:
            n += l.upper()
            cap = False
        elif l == '_':
            cap = True
        else:
            n += l
    return n

class Member(object):
    def __init__(self, msg, id, name, type, default, comment):
        self.msg = msg
        self.id = id
        self.name = name
        self.func_name = toCamelCase(name, True)
        self.type = None
        self.default = default
        self.comment = comment
        self.is_arr = False
        self.struct = None
        self.macro_name = '{0}_{1}'.format(msg.name.upper(), name.upper())

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
            line = line.format(self.struct.struct_name,
                               asterix, self.name, comm)
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

    def cSchema(self):
        foffset = '_BOR_MSG_SCHEMA_OFFSET({0}, {{0}})' \
                        .format(self.msg.struct_name)

        stype = '_BOR_MSG_SCHEMA_' + self.type.upper()
        soffset = foffset.format(self.name)
        ssize_offset = '-1'
        salloc_offset = '-1'
        ssub = 'NULL'
        sdefault = '(void *)(((char *)&___{0}_default) + {1})' \
                        .format(self.msg.name, soffset)

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

    def cHeaderMacro(self):
        s = '#define {0} {1}'
        s = s.format(self.macro_name, self.id)
        return s

    def cFuncStruct(self):
        s = '''_bor_inline const {0} *{1}Get{2}(const {3} *msg)
{{
    return &msg->{4};
}}

_bor_inline {0} *{1}Set{2}({3} *msg)
{{
    borMsgSetHeaderField(msg, {5}_schema, {6});
    return &msg->{4};
}}
_bor_inline void {1}Unset{2}({3} *msg)
{{
    borMsgUnsetHeaderField(msg, {5}_schema, {6});
}}
'''
        s = s.format(self.struct.struct_name,
                     self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     self.name,
                     self.msg.name,
                     self.macro_name,
                     self.struct.func_prefix)
        return s

    def cFuncArr(self):
        s = '''_bor_inline int {0}Get{1}Size(const {2} *msg)
{{
    return msg->{3}_size;
}}
_bor_inline int {0}Get{1}Alloc(const {2} *msg)
{{
    return msg->{3}_alloc;
}}
_bor_inline {4} {0}GetElem{1}(const {2} *msg, int idx)
{{
    return msg->{3}[idx];
}}

_bor_inline const {4} *{0}Get{1}(const {2} *msg, int *size)
{{
    if (size != NULL)
        *size = msg->{3}_size;
    return msg->{3};
}}
#define {5}_{6}_FOR_EACH(MSG, VAL) \\
    for (int i = 0; i < (MSG)->{3}_size && ((VAL) = (MSG)->{3}[i], 1); ++i)

void {0}Add{1}({2} *msg, {4} val);
void {0}Remove{1}({2} *msg, int idx);
_bor_inline void {0}SetElem{1}({2} *msg, int idx, {4} val)
{{
    borMsgSetHeaderField(msg, {7}_schema, {8});
    msg->{3}[idx] = val;
}}
_bor_inline {4} *{0}Set{1}({2} *msg, int *size)
{{
    borMsgSetHeaderField(msg, {7}_schema, {8});
    if (size != NULL)
        *size = msg->{3}_size;
    return msg->{3};
}}
void {0}SetArr{1}({2} *msg, const {4} *arr, int size);
_bor_inline void {0}Unset{1}({2} *msg)
{{
    borMsgUnsetHeaderField(msg, {7}_schema, {8});
}}
void {0}Reserve{1}({2} *msg, int size);
void {0}Resize{1}({2} *msg, int size);
'''
        s = s.format(self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     self.name,
                     TYPES[self.type],
                     self.msg.name.upper(),
                     self.name.upper(),
                     self.msg.name,
                     self.macro_name)
        return s

    def cFuncArrImpl(self):
        s = '''void {0}Add{1}({2} *msg, {3} val)
{{
    if (msg->{4}_size == msg->{4}_alloc){{
        if (msg->{4}_alloc == 0)
            msg->{4}_alloc = 1;
        msg->{4}_alloc *= 2;
        msg->{4} = BOR_REALLOC_ARR(msg->{4}, {3}, msg->{4}_alloc);
    }}
    borMsgSetHeaderField(msg, {5}_schema, {6});
    msg->{4}[msg->{4}_size++] = val;
}}
void {0}Remove{1}({2} *msg, int idx)
{{
    for (int i = idx + 1; i < msg->{4}_size; ++i)
        msg->{4}[i - 1] = msg->{4}[i];
    --msg->{4}_size;
    if (msg->{4}_size == 0)
        borMsgUnsetHeaderField(msg, {5}_schema, {6});
}}
void {0}SetArr{1}({2} *msg, const {3} *arr, int size)
{{
    if (msg->{4}_alloc < size){{
        msg->{4}_alloc = size;
        msg->{4} = BOR_REALLOC_ARR(msg->{4}, {3}, msg->{4}_alloc);
    }}
    borMsgSetHeaderField(msg, {5}_schema, {6});
    memcpy(msg->{4}, arr, sizeof({3}) * size);
    msg->{4}_size = size;
}}
void {0}Reserve{1}({2} *msg, int size)
{{
    if (msg->{4}_alloc >= size)
        return;
    msg->{4}_alloc = size;
    msg->{4} = BOR_REALLOC_ARR(msg->{4}, {3}, msg->{4}_alloc);
}}
void {0}Resize{1}({2} *msg, int size)
{{
    borMsgSetHeaderField(msg, {5}_schema, {6});
    {0}Reserve{1}(msg, size);
    msg->{4}_size = size;
}}
'''
        s = s.format(self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     TYPES[self.type],
                     self.name,
                     self.msg.name,
                     self.macro_name)
        return s

    def cFuncArrStruct(self):
        s = '''_bor_inline int {0}Get{1}Size(const {2} *msg)
{{
    return msg->{3}_size;
}}
_bor_inline int {0}Get{1}Alloc(const {2} *msg)
{{
    return msg->{3}_alloc;
}}
_bor_inline const {4} *{0}GetElem{1}(const {2} *msg, int idx)
{{
    return msg->{3} + idx;
}}

_bor_inline const {4} *{0}Get{1}(const {2} *msg, int *size)
{{
    *size = msg->{3}_size;
    return msg->{3};
}}
#define {5}_{6}_FOR_EACH(MSG, VAL) \\
    for (int i = 0; i < (MSG)->{3}_size && ((VAL) = (MSG)->{3} + i, 1); ++i)

{4} *{0}Add{1}({2} *msg);
void {0}Remove{1}({2} *msg, int idx);
{4} *{0}SetElem{1}({2} *msg, int idx);
_bor_inline {4} *{0}Set{1}({2} *msg, int *size)
{{
    borMsgSetHeaderField(msg, {7}_schema, {8});
    *size = msg->{3}_size;
    return msg->{3};
}}
_bor_inline void {0}Unset{1}({2} *msg)
{{
    borMsgUnsetHeaderField(msg, {7}_schema, {8});
}}
void {0}Reserve{1}({2} *msg, int size);
void {0}Resize{1}({2} *msg, int size);
'''
        s = s.format(self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     self.name,
                     self.struct.struct_name,
                     self.msg.name.upper(),
                     self.name.upper(),
                     self.msg.name,
                     self.macro_name)
        return s

    def cFuncArrStructImpl(self):
        s = '''{3} *{0}Add{1}({2} *msg)
{{
    if (msg->{4}_size == msg->{4}_alloc){{
        if (msg->{4}_alloc == 0)
            msg->{4}_alloc = 1;
        msg->{4}_alloc *= 2;
        msg->{4} = BOR_REALLOC_ARR(msg->{4}, {3}, msg->{4}_alloc);
        for (int i = msg->{4}_size; i < msg->{4}_alloc; ++i)
            {5}Init(msg->{4} + i);
    }}
    borMsgSetHeaderField(msg, {6}_schema, {7});
    return msg->{4} + msg->{4}_size++;
}}
void {0}Remove{1}({2} *msg, int idx)
{{
    {5}Free(msg->{4} + idx);
    for (int i = idx + 1; i < msg->{4}_size; ++i)
        msg->{4}[i - 1] = msg->{4}[i];
    --msg->{4}_size;
    if (msg->{4}_size == 0)
        borMsgUnsetHeaderField(msg, {6}_schema, {7});
}}
{3} *{0}SetElem{1}({2} *msg, int idx)
{{
    borMsgSetHeaderField(msg, {6}_schema, {7});
    return msg->{4} + idx;
}}
void {0}Reserve{1}({2} *msg, int size)
{{
    if (msg->{4}_alloc >= size)
        return;
    msg->{4}_alloc = size;
    msg->{4} = BOR_REALLOC_ARR(msg->{4}, {3}, msg->{4}_alloc);
}}
void {0}Resize{1}({2} *msg, int size)
{{
    borMsgSetHeaderField(msg, {6}_schema, {7});
    {0}Reserve{1}(msg, size);
    for (int i = msg->{4}_size; i < size; ++i)
        {5}Init(msg->{4} + i);
    msg->{4}_size = size;
}}
'''
        s = s.format(self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     self.struct.struct_name,
                     self.name,
                     self.struct.func_prefix,
                     self.msg.name,
                     self.macro_name)
        return s

    def cFunc(self):
        if self.is_arr and self.type == 'struct':
            return self.cFuncArrStruct()
        if self.is_arr:
            return self.cFuncArr()
        if self.type == 'struct':
            return self.cFuncStruct()

        s = '''_bor_inline {3} {0}Get{1}(const {2} *msg)
{{
    return msg->{4};
}}

_bor_inline void {0}Set{1}({2} *msg, {3} val)
{{
    borMsgSetHeaderField(msg, {5}_schema, {6});
    msg->{4} = val;
}}
_bor_inline void {0}Unset{1}({2} *msg)
{{
    borMsgUnsetHeaderField(msg, {5}_schema, {6});
}}
'''
        s = s.format(self.msg.func_prefix,
                     self.func_name,
                     self.msg.struct_name,
                     TYPES[self.type],
                     self.name,
                     self.msg.name,
                     self.macro_name)
        return s

    def cFuncImpl(self):
        if self.is_arr and self.type == 'struct':
            return self.cFuncArrStructImpl()
        if self.is_arr:
            return self.cFuncArrImpl()
        return ''

class Msg(object):
    def __init__(self, name):
        self.name = name
        self.struct_name = name + '_t'
        self.func_prefix = toCamelCase(name)
        self.members = []
        self.before = ''
        self.after = ''

        STRUCTS[self.name] = self

    def addMember(self, name, type, default = None, comment = None):
        if len(self.members) == MAX_MEMBERS:
            print('Error: Exceeded maximal number of struct members ({0})!'
                    .format(MAX_MEMBERS), file = sys.stderr)
            sys.exit(-1)
        m = Member(self, len(self.members), name, type, default, comment)
        self.members += [m]

    def finalize(self, before):
        self.before = before

    def addAfter(self, after):
        self.after = after

    def genCStruct(self, fout):
        fout.write(self.before)
        fout.write('struct _{0} {{\n'.format(self.struct_name))
        fout.write('    {0} __msg_header;\n'.format(HEADER_TYPE))
        for m in self.members:
            m.genCStructMember(fout)
        fout.write('};\n')
        fout.write('typedef struct _{0} {0};\n'.format(self.struct_name))
        fout.write('extern bor_msg_schema_t *{0}_schema;\n'.format(self.name))
        fout.write(self.after)

    def cDefaultVal(self):
        val = [m.cDefaultVal() for m in self.members]
        val = '{ 0, ' + ', '.join(val) + ' }'
        return val

    def genCDefault(self, fout):
        default = [m.cDefaultVal() for m in self.members]
        fout.write('static {0} ___{1}_default = ' \
                        .format(self.struct_name, self.name))
        fout.write(self.cDefaultVal())
        fout.write(';\n')

    def genCSchema(self, fout):
        fields = [m.cSchema() for m in self.members]
        fields = ['    ' + x for x in fields]
        fields = ',\n'.join(fields)

        fout.write('static bor_msg_schema_field_t ___{0}_fields[] = {{\n'.format(self.name))
        fout.write(fields + '\n')
        fout.write('};\n');
        fout.write('static bor_msg_schema_t ___{0}_schema = {{\n'.format(self.name))
        fout.write('    0,\n')
        fout.write('    sizeof({0}),\n'.format(self.struct_name))
        fout.write('    ___{0}_fields,\n'.format(self.name))
        fout.write('    sizeof(___{0}_fields) / sizeof(bor_msg_schema_field_t),\n'.format(self.name))
        fout.write('    &___{0}_default\n'.format(self.name))
        fout.write('};\n');
        fout.write('bor_msg_schema_t *{0}_schema = &___{0}_schema;\n'.format(self.name))
        fout.write('\n')

    def genCHeaderMacros(self, fout):
        f = [m.cHeaderMacro() for m in self.members]
        fout.write('\n'.join(f))
        fout.write('\n\n')

    def genCFunc(self, fout):
        s = '''
_bor_inline void {0}Init({1} *msg)
{{
    borMsgInit(msg, {2}_schema);
}}
_bor_inline void {0}Free({1} *msg)
{{
    borMsgFree(msg, {2}_schema);
}}
_bor_inline {1} *{0}New(void)
{{
    return borMsgNew({2}_schema);
}}
_bor_inline void {0}Del({1} *msg)
{{
    borMsgDel(msg, {2}_schema);
}}
_bor_inline void {0}SetHeader({1} *msg)
{{
    borMsgSetHeader(msg, {2}_schema);
}}
_bor_inline int {0}Encode(const {1} *msg, unsigned char **buf, int *bufsize)
{{
    return borMsgEncode(msg, {2}_schema, buf, bufsize);
}}
_bor_inline int {0}Decode(unsigned char *buf, int bufsize, {1} *msg)
{{
    return borMsgDecode(buf, bufsize, msg, {2}_schema);
}}
'''
        s = s.format(self.func_prefix,
                     self.struct_name,
                     self.name)
        for m in self.members:
            s += m.cFunc()
        fout.write(s)

    def genCFuncImpl(self, fout):
        s = ''
        for m in self.members:
            s += m.cFuncImpl()
        fout.write(s)

def parseMsgs():
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
            s = Msg(sline[1])
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

    for s in structs:
        s.genCFunc(fout)

def genCC(structs, fout):
    for s in structs:
        s.genCDefault(fout)

    fout.write('\n')
    for s in structs:
        s.genCSchema(fout)

    fout.write('\n')
    for s in structs:
        s.genCFuncImpl(fout)

if __name__ == '__main__':
    opts = ['--h', '--c']
    if len(sys.argv) != 2 or sys.argv[1] not in opts:
        print('Usage: {0} [--h|--c] <in.sch'.format(sys.argv[0]))
        sys.exit(-1)

    structs = parseMsgs()
    if sys.argv[1] == '--h':
        genCH(structs, sys.stdout)
    if sys.argv[1] == '--c':
        genCC(structs, sys.stdout)
