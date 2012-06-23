from random import uniform
from cgkit.cgtypes import mat4, vec4

LEN = 3000

mat4s = []
for i in range(0, LEN):
    m = [0] * 16
    for j in range(0, 16):
        m[j] = uniform(-2, 2)
    mat4s.append(m)


print('#include <boruvka/mat4.h>')
print('')

print('bor_mat4_t mat4s[] = {')
for m in mat4s:
    print('    BOR_MAT4_STATIC({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15}),'.format(*m))
print('};')
print('size_t mat4s_len = sizeof(mat4s) / sizeof(bor_mat4_t);')

print('')
print('')

def pr(m, p = ''):
    a = m.toList(True)
    print('# {0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12} {13} {14} {15} {16}'.format(p, *a))

print('# ---- add sub ----')
for i in range(0, LEN - 1):
    a = mat4(*mat4s[i])
    b = mat4(*mat4s[i + 1])
    pr(a + b, 'add')
    pr(a - b, 'sub')
print('# ---- add sub end ----')
print('')

print('# ---- const ----')
for i in range(0, LEN):
    a = mat4(*mat4s[i])
    c = mat4s[i][0]
    pr(a * c, 'scale')

    d = mat4(c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c)
    pr(a + d, 'add')
print('# ---- const end ----')
print('')

print('# ---- mul ----')
for i in range(0, LEN - 1):
    a = mat4(*mat4s[i])
    b = mat4(*mat4s[i + 1])
    pr(a * b, 'right')
    pr(b * a, 'left')
print('# ---- mul end ----')
print('')

print('# ---- trans ----')
for i in range(0, LEN):
    a = mat4(*mat4s[i])
    pr(a.transpose())
print('# ---- trans end ----')
print('')

print('# ---- det ----')
for i in range(0, LEN):
    a = mat4(*mat4s[i])
    print('# {0}'.format(a.determinant()))
print('# ---- det end ----')
print('')

print('# ---- inv ----')
for i in range(0, LEN):
    a = mat4(*mat4s[i])
    inv = a.inverse()
    try:
        pr(inv)
    except:
        print('# 0')
print('# ---- inv end ----')
print('')

print('# ---- mul vec ----')
for i in range(0, LEN):
    a = mat4(*mat4s[i])
    v = vec4(mat4s[i][1], mat4s[i][2], mat4s[i][3], mat4s[i][4])
    w = a * v
    u = vec4(mat4s[i][3], mat4s[i][4], mat4s[i][5], 1)
    u = a * u
    u = u / u.w
    print('# vec4 {0} {1} {2} {3}'.format(w.x, w.y, w.z, w.w))
    print('# vec3 {0} {1} {2}'.format(u.x, u.y, u.z))
print('# ---- mul vec end ----')
print('')
