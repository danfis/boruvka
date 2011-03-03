import unittest
from common import *
from fermat import *


class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Vec4')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Vec4()
        self.out(a)
        a = Vec4(1)
        self.out(a)
        a = Vec4(1, 2)
        self.out(a)
        a = Vec4(1, 2, 3)
        self.out(a)
        a = Vec4(1, 2, 3, 4)
        self.out(a)

        self.out()

        b = Vec4(3.1, 1.2, 0.4, 11.2)
        self.out(str(a), '+', str(b), '=', str(a + b))
        self.out(str(a), '+', 1., '=', str(a + 1.))
        self.out(1., '+', str(a), '=', str(1. + a))

        self.out(str(a), '-', str(b), '=', str(a - b))
        self.out(str(a), '-', 1., '=', str(a - 1.))

        self.out(str(a), '*', str(b), '=', str(a * b))
        self.out(str(a), '*', 1.5, '=', str(a * 1.5))
        self.out(1.5, '*', str(a), '=', str(1.5 * a))

        self.out(str(-a))

        self.out(str(a), '/', 2, '=', str(a / 2))

        self.out()
        b = Vec4(3.1, 1.2, 0.4)
        b += a
        self.out(str(b))
        b += 10.
        self.out(str(b))
        b -= a
        self.out(str(b))
        b -= 10
        self.out(str(b))
        b *= 2.
        self.out(str(b))
        b /= 2.
        self.out(str(b))

    def testSeqProtocol(self):
        self.out('---- SeqProtocol ----')
        a = Vec4(1, 2, 3, 4)
        self.out(len(a), a[0], a[1], a[2], a[3])
        self.out(a.x, a.y, a.z, a.w)
        for i in a:
            self.out(i)

    def testCmp(self):
        self.assertTrue(Vec4(1, 2, 3, 4) == Vec4(1, 2, 3, 4))
        self.assertTrue(Vec4(1, 2, 3, 4) != Vec4(2, 2, 3, 4))

    def testMethods(self):
        self.out('---- Methods ----')
        a = Vec4(1, 2, 3)
        b = a.copy()
        b += 1
        self.out(a, b)

        self.out('len', a.len())
        self.out('len2', a.len2())
        self.out('dist', a.dist(b))
        self.out('dist2', a.dist2(b))
        b.scaleToLen(4.)
        self.out('scaleToLen', b, b.len())
        self.out('scaledToLen', b.scaledToLen(2.), b.len(), b.scaledToLen(2.).len())
        self.out('normalized', b.normalized(), b.len(), b.normalized().len())
        b.normalize()
        self.out('normalize', b, b.len())
