#!/usr/bin/env python

import mg
import math

v = mg.Vec3()
print('repr', repr(v))
print('str', str(v))
v.x = 10
v.y = 1
v.z = 2
print(v, v.x, v.y, v.z)
print(v[0])
print(v[1])
print(v[2])
v[0] = 4.
print(v)

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
print(v / 3)
print(v // 3)
print(-v)

print()
print(v)
v *= 1.4
print(v)
v /= 1.4
print(v)
v //= 1.4
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

print()
r = v.segmentDist2(u, w);
print(v, u, w, r, math.sqrt(r[0]))

print()
print("Vec4")
v = mg.Vec4()
print(v)
v.x = 1.
v.y = 2
v.z = 3.
v.w = 4
print(v)
w = mg.Vec4(3, 1.2, 3, 5)
print(w)

print()
v = v + w
print(v)
v = w - v
print(v)
print(v * w)
print(v * 2.)
print(-v)
v += w
print(v)
v -= w
print(v)
v /= 10
print(v)

print()
print(v.len2())
print(v.len())
print(v.dist2(w))
print(v.dist(w))
v.scaleToLen(2.)
print(v)
print(w.scaledToLen(10.), w)
v.normalize()
print(v)
print(w.normalized(), w)



print()
print("Quat")
v = mg.Quat()
print(v)
v.x = 1.
v.y = 2
v.z = 3.
v.w = 4
print(v)
w = mg.Quat(3, 1.2, 3, 5)
print(w)

print(v * 2.)
print(-v)
v /= 10
print(v)

print()
print(v.len2())
print(v.len())
v.normalize()
print(v)
print(w.normalized(), w)

print()
print(v)
v.setAngleAxis(math.pi / 2., mg.Vec3(1., 1., 1.))
print(v)
w.setAngleAxis(math.pi / 2., mg.Vec3(1., 0., 0.))
print(w)
vv = v.copy()
v.mul(w)
print(v)
print(vv.muled(w), vv)

vv = v.copy()
v.invert()
print(v)
print(vv.inverted(), vv)

vec = mg.Vec3(1., 2., 3.)
vec2 = vec.copy()
v.rot(vec)
print(v, vec)
print(v, v.roted(vec2))


