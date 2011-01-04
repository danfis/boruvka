from random import uniform
from cgkit.cgtypes import mat3, vec3

LEN = 10000

mat3s = []
for i in range(0, LEN):
    m = [0] * 9
    for j in range(0, 9):
        m[j] = uniform(-2, 2)
    mat3s.append(m)


print('#include <fermat/mat3.h>')
print('')

print('fer_mat3_t mat3s[] = {')
for m in mat3s:
    print('    FER_MAT3_STATIC({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}),'.format(*m))
print('};')
print('size_t mat3s_len = sizeof(mat3s) / sizeof(fer_mat3_t);')

print('')
print('')

def pr(m, p = ''):
    a = m.toList(True)
    print('# {0} {1} {2} {3} {4} {5} {6} {7} {8} {9}'.format(p, *a))

print('# ---- add sub ----')
for i in range(0, LEN - 1):
    a = mat3(*mat3s[i])
    b = mat3(*mat3s[i + 1])
    pr(a + b, 'add')
    pr(a - b, 'sub')
print('# ---- add sub end ----')
print('')

print('# ---- const ----')
for i in range(0, LEN):
    a = mat3(*mat3s[i])
    c = mat3s[i][0]
    pr(a * c, 'scale')

    d = mat3(c, c, c, c, c, c, c, c, c)
    pr(a + d, 'add')
print('# ---- const end ----')
print('')

print('# ---- mul ----')
for i in range(0, LEN - 1):
    a = mat3(*mat3s[i])
    b = mat3(*mat3s[i + 1])
    pr(a * b, 'right')
    pr(b * a, 'left')
print('# ---- mul end ----')
print('')

print('# ---- trans ----')
for i in range(0, LEN):
    a = mat3(*mat3s[i])
    pr(a.transpose())
print('# ---- trans end ----')
print('')

print('# ---- det ----')
for i in range(0, LEN):
    a = mat3(*mat3s[i])
    print('# {0}'.format(a.determinant()))
print('# ---- det end ----')
print('')

print('# ---- inv ----')
for i in range(0, LEN):
    a = mat3(*mat3s[i])
    inv = a.inverse()
    try:
        pr(inv)
    except:
        print('# 0')
print('# ---- inv end ----')
print('')

print('# ---- mul vec ----')
for i in range(0, LEN):
    a = mat3(*mat3s[i])
    v = vec3(mat3s[i][1], mat3s[i][2], mat3s[i][3])
    w = a * v
    u = vec3(mat3s[i][3], mat3s[i][4], 1)
    u = a * u
    u = u / u.z
    print('# vec3 {0} {1} {2}'.format(w.x, w.y, w.z))
    print('# vec2 {0} {1}'.format(u.x, u.y))
print('# ---- mul vec end ----')
print('')
