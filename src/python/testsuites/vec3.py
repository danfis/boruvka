import unittest
from common import *
from fermat import *

class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Vec3')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Vec3()
        self.out(a)
        a = Vec3(1)
        self.out(a)
        a = Vec3(1, 2)
        self.out(a)
        a = Vec3(1, 2, 3)
        self.out(a)

        self.out()

        b = Vec3(3.1, 1.2, 0.4)
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
        b = Vec3(3.1, 1.2, 0.4)
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
        a = Vec3(1, 2, 3)
        self.out(len(a), a[0], a[1], a[2])
        self.out(a.x, a.y, a.z)
        for i in a:
            self.out(i)

    def testCmp(self):
        self.assertTrue(Vec3(1, 2, 3) == Vec3(1, 2, 3))
        self.assertTrue(Vec3(1, 2, 3) != Vec3(2, 2, 3))

    def testMethods(self):
        self.out('---- Methods ----')
        a = Vec3(1, 2, 3)
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
        self.out('cross', a, b, a.cross(b))

        self.out()
        self.out('segmentDist2', Vec3().segmentDist2(Vec3(1, 0, 0), Vec3(2, 0, 0)))
        self.out('triDist2', Vec3().triDist2(Vec3(1, 0, 0), Vec3(2, 0, 0), Vec3(2, 1, 0)))
        self.out('inTri', Vec3().inTri(Vec3(1, 0, 0), Vec3(2, 0, 0), Vec3(2, 1, 0)))
        self.out('inTri', Vec3().inTri(Vec3(-1, 0, 0), Vec3(2, -1, 0), Vec3(2, 1, 0)))
        self.out('angle', Vec3.angle(Vec3(-1, 0, 0), Vec3(2, -1, 0), Vec3(2, 1, 0)))
        self.out('angle', Vec3.angle(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0)))
        self.out('dihedralAngle', Vec3.dihedralAngle(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0), Vec3(0, 0, 1)))
        self.out('projToPlane', Vec3(0, 0).projToPlane(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0)))
        self.out('projToPlane2', Vec3(0, 0).projToPlane2(Vec3(0, 0, -1), Vec3(0, 0, 1)))
        self.out('triArea2', Vec3.triArea2(Vec3(0, 0, 0), Vec3(0, 0, 1), Vec3(0, 1, 0)))
