#!/usr/bin/env python

import mg

v = mg.Vec3()
print('repr', repr(v))
print('str', str(v))
v.x = 10
v.y = 1
v.z = 2
print(v, v.x, v.y, v.z)

print()
v = mg.Vec3(1)
print(str(v))
v = mg.Vec3(1, 2)
print(str(v))
v = mg.Vec3(1, 2, 3)
print(str(v))

print()
w = mg.Vec3(1, 2, 3)
print(v == v)
print(v == w)
print(v != v)
print(v != w)

u = mg.Vec3(2, 3, 1)
print()
print(v.len2())
print(v.len())
print(v.dist2(u))
print(v.dist(u))

print()
print(v + w)
print(v + u)
print(v - w)
print(v - u)
print(v * w)
print(v * u)
print(v * 2.)
print(v * 3)
print(-v)

print()
print(v)
v *= 1.4
print(v)
v += v
print(v)
v += w
print(v)
v -= v
print(v)
v -= w
print(v)

print()
v2 = v.normalized()
print(v2, v)
v.normalize()
print(v)

print()
v2 = v.cross(u)
print(v, u, v2)
