#!/usr/bin/env python

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

