import unittest
import math
from common import *
from fermat import *


class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Mat4')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Mat4()
        self.out(a)
        a = Mat4(1)
        self.out(a)
        a = Mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
        self.out(a)

        v2 = Vec3(2, 3)
        v3 = Vec4(1, 2, 3)
        self.out()
        b = Mat4(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3, 3, 4, 5, 6, 7, 8, 9)
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
        b = Mat4(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3, 3, 1, 2, 7, 5, 3, 4)
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
        a = Mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
        self.out(len(a), a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15])
        for i in a:
            self.out(i)

    def testMethods(self):
        self.out('---- Methods ----')
        a = Mat4()
        a.setIdentity()
        self.out(a)
        a.setZero()
        self.out(a)
        a.setScale(1.1)
        self.out(a)
        a.setTranslate(Vec3(1, 2, 3))
        self.out(a)
        a.setRot(math.pi / 3, Vec3(1, 1, 1))
        self.out(a)
        a.setRot(Vec3(1, 1, 1), math.pi / 3)
        self.out(a)

        a.trScale(3.)
        self.out(a)
        a.translate(Vec3(1, 2, 3))
        self.out(a)
        a.rot(-math.pi / 3, Vec3(1, 2, 3))

        self.out()
        b = Mat4()
        b.setTranslate(Vec3(4, 3, 5))
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
