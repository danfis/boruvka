import unittest
from common import *
from fermat import *

def st(v):
    s = '['
    for a in v:
        s += ' {0}'.format(a)
    s += ' ]'
    return s

class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Vec')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Vec(5)
        self.out(a)
        a[0] = 1
        a[1] = 12
        a[2] = 3
        a[3] = 4
        a[4] = .1

        b = Vec(5)
        b[0] = 2
        b[1] = 22
        b[2] = 4
        b[3] = 2
        b[4] = .3
        self.out()

        self.out(st(a), '+', st(b), '=', st(a + b))
        self.out(st(a), '+', 1., '=', st(a + 1.))
        self.out(1., '+', st(a), '=', st(1. + a))

        self.out(st(a), '-', st(b), '=', st(a - b))
        self.out(st(a), '-', 1., '=', st(a - 1.))

        self.out(st(a), '*', st(b), '=', str(a * b))
        self.out(st(a), '*', 1.5, '=', st(a * 1.5))
        self.out(1.5, '*', st(a), '=', st(1.5 * a))

        self.out(st(-a))

        self.out(st(a), '/', 2, '=', st(a / 2))

        self.out()
        b += a
        self.out(st(b))
        b += 10.
        self.out(st(b))
        b -= a
        self.out(st(b))
        b -= 10
        self.out(st(b))
        b *= 2.
        self.out(st(b))
        b /= 2.
        self.out(st(b))

    def testSeqProtocol(self):
        self.out('---- SeqProtocol ----')
        a = Vec(10)
        for i in range(0, 10):
            a[i] = i
        for i in a:
            self.out(i)

    def testMethods(self):
        self.out('---- Methods ----')
        a = Vec(3)
        a[0] = 1
        a[1] = 2
        a[2] = 3
        b = a.copy()
        b += 1
        self.out(a, b)
        self.out(st(a), st(b))

        self.out('len', a.len())
        self.out('len2', a.len2())
        self.out('dist', a.dist(b))
        self.out('dist2', a.dist2(b))
