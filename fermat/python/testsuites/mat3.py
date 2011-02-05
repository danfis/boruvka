import unittest
import math
from common import *
from fermat import *


class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Mat3')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Mat3()
        self.out(a)
        a = Mat3(1)
        self.out(a)
        a = Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9)
        self.out(a)

        v2 = Vec2(2, 3)
        v3 = Vec3(1, 2, 3)
        self.out()
        b = Mat3(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3)
        self.out(str(a), '+', 1.5, '=', str(a + 1.5))
        self.out(1.5, '+', str(a), '=', str(1.5 + a))
        self.out(str(a), '-', 1.5, '=', str(a - 1.5))
        self.out(str(a), '*', 1.5, '=', str(a * 1.5))
        self.out(1.5, '*', str(a), '=', str(1.5 * a))
        self.out(a, '*', b, '=', a * b)
        self.out(a, '*', v2, '=', a * v2)
        self.out(a, '*', v3, '=', a * v3)

        self.out(-a)

        self.out(a, '/', 2, '=', a / 2)

        self.out()
        b = Mat3(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3)
        self.out(b)
        b *= 2.
        self.out(b)
        b /= 2.
        self.out(b)
        b += 2.
        self.out(b)
        b -= 2.
        self.out(b)

    def testSeqProtocol(self):
        self.out('---- SeqProtocol ----')
        a = Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9)
        self.out(len(a), a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8])
        for i in a:
            self.out(i)

    def testMethods(self):
        self.out('---- Methods ----')
        a = Mat3()
        a.setIdentity()
        self.out(a)
        a.setZero()
        self.out(a)
        a.setScale(1.1)
        self.out(a)
        a.setTranslate(Vec2(1, 2))
        self.out(a)
        a.setRot(math.pi / 3)
        self.out(a)

        a.trScale(3.)
        self.out(a)
        a.translate(Vec2(1, 2))
        self.out(a)
        a.rot(-math.pi / 3)

        self.out()
        b = Mat3()
        b.setTranslate(Vec2(4, 3))
        self.out(a.composed(b))
        a.compose(b)
        self.out(a)

        self.out(a.muledComp(b))
        a.mulComp(b)
        self.out(a)

        self.out(a.transposed())
        a.transpose()
        self.out(a)

        self.out(a.regular())
        self.out(a.singular())
        self.out(a.det())
        self.out(a.inved())
        a.inv()
        self.out(a)

        self.out()
        self.out(a)
        c = a.copy()
        c += 10
        self.out(a)
        self.out(c)
