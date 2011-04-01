#!/usr/bin/env python

from time import sleep
import fermat
print('dir(fermat):', dir(fermat))

print('EPS:', fermat.EPS)
print('REAL_MAX:', fermat.REAL_MAX)
print('REAL_MIN:', fermat.REAL_MIN)
print('ONE:', fermat.ONE)
print('ZERO:', fermat.ZERO)
print('')

print('sign(-10):', fermat.sign(-10))
print('sign(10):', fermat.sign(10))
print('isZero(10):', fermat.isZero(10))
print('isZero(0):', fermat.isZero(0))
print('eq(0, 1):', fermat.eq(0, 1))
print('eq(1, 1):', fermat.eq(1, 1))
print('neq(0, 1):', fermat.neq(0, 1))
print('neq(1, 1):', fermat.neq(1, 1))
print('recp(10):', fermat.recp(10))
print('rsqrt(4):', fermat.rsqrt(4))
print('swap(10, 11)', fermat.swap(10, 11))
print('')


print('Rand:')
r = fermat.Rand()
print('r.uniform(0, 1):', r.uniform(0, 1))
print('r.uniform(-10, 1):', r.uniform(-10, 1))
print('')


print('RandMT:')
r = fermat.RandMT()
print('r.uniform(0, 1):', r.uniform(0, 1))
print('r.uniform(-10, 1):', r.uniform(-10, 1))
print('r.normal(0, 1):', r.normal(0, 1))
print('r.normal(-10, 1):', r.normal(-10, 1))
print('')

print('Timer:')
t = fermat.Timer()
t.start()
sleep(1)
t.stop()
print('t.ns()', t.ns())
print('t.us()', t.us())
print('t.ms()', t.ms())
print('t.s()', t.s())
print('t.m()', t.m())
print('t.h()', t.h())

t.start()
sleep(1)
t.stop()
print('t.inNs()', t.inNs())
print('t.inUs()', t.inUs())
print('t.inMs()', t.inMs())
print('t.inS()', t.inS())
print('t.inM()', t.inM())
print('t.inH()', t.inH())
print('')
