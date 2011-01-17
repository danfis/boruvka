import unittest
import math
from common import *
from fermat import *


class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Quat')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Quat()
        self.out(a)
        a = Quat(1)
        self.out(a)
        a = Quat(1, 2)
        self.out(a)
        a = Quat(1, 2, 3)
        self.out(a)
        a = Quat(1, 2, 3, 4)
        self.out(a)

        self.out()

        b = Quat(3.1, 1.2, 0.4, 11.2)
        self.out(str(a), '*', 1.5, '=', str(a * 1.5))
        self.out(1.5, '*', str(a), '=', str(1.5 * a))

        self.out(str(-a))

        self.out(str(a), '/', 2, '=', str(a / 2))

        self.out()
        b = Quat(3.1, 1.2, 0.4)
        b *= 2.
        self.out(str(b))
        b /= 2.
        self.out(str(b))

        self.out()

        a = Quat()
        v = Vec3(1., 1., 1.)
        a.setAngleAxis(math.pi, Vec3(1, 0, 0))
        self.out(a, a.inverted())
        self.out(a)
        self.out(v)
        a.rot(v)
        self.out(v)
        a.invert()
        self.out(a.roted(v))

        self.out()
        b = Quat().setAngleAxis(math.pi, Vec3(0, 1, 0))
        self.out(b)
        self.out(a.muled(b))
        self.out(a.muled(b).roted(v))

    def testSeqProtocol(self):
        self.out('---- SeqProtocol ----')
        a = Quat(1, 2, 3, 4)
        self.out(len(a), a[0], a[1], a[2], a[3])
        self.out(a.x, a.y, a.z, a.w)
        for i in a:
            self.out(i)

    def testCmp(self):
        self.assertTrue(Quat(1, 2, 3, 4) == Quat(1, 2, 3, 4))
        self.assertTrue(Quat(1, 2, 3, 4) != Quat(2, 2, 3, 4))

    def testMethods(self):
        self.out('---- Methods ----')
        a = Quat(1, 2, 3, 4)
        b = a.copy()
        b *= 2
        self.out(a, b)

        self.out('len', a.len())
        self.out('len2', a.len2())
        self.out('normalized', b.normalized(), b.len(), b.normalized().len())
        b.normalize()
        self.out('normalize', b, b.len())
