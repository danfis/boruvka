import unittest
from fermat import *

fout = None
def out(*a):
    s = ''
    for i in a:
        s += str(i) + ' '
    fout.write(s)
    fout.write('\n')

class Vec3TestCase(unittest.TestCase):
    def testNumberProtocol(self):
        out('---- NumberProtocol ----')
        a = Vec3()
        out(a)
        a = Vec3(1)
        out(a)
        a = Vec3(1, 2)
        out(a)
        a = Vec3(1, 2, 3)
        out(a)

        out()

        b = Vec3(3.1, 1.2, 0.4)
        out(str(a), '+', str(b), '=', str(a + b))
        out(str(a), '+', 1., '=', str(a + 1.))
        out(1., '+', str(a), '=', str(1. + a))

        out(str(a), '-', str(b), '=', str(a - b))
        out(str(a), '-', 1., '=', str(a - 1.))

        out(str(a), '*', str(b), '=', str(a * b))
        out(str(a), '*', 1.5, '=', str(a * 1.5))
        out(1.5, '*', str(a), '=', str(1.5 * a))

        out(str(-a))

        out(str(a), '/', 2, '=', str(a / 2))

        out()
        b = Vec3(3.1, 1.2, 0.4)
        b += a
        out(str(b))
        b += 10.
        out(str(b))
        b -= a
        out(str(b))
        b -= 10
        out(str(b))
        b *= 2.
        out(str(b))
        b /= 2.
        out(str(b))

    def testSeqProtocol(self):
        out('---- SeqProtocol ----')
        a = Vec3(1, 2, 3)
        out(len(a), a[0], a[1], a[2])
        out(a.x, a.y, a.z)
        for i in a:
            out(i)

    def testCmp(self):
        self.assertTrue(Vec3(1, 2, 3) == Vec3(1, 2, 3))
        self.assertTrue(Vec3(1, 2, 3) != Vec3(2, 2, 3))

    def testMethods(self):
        out('---- Methods ----')
        a = Vec3(1, 2, 3)
        b = a.copy()
        b += 1
        out(a, b)

        out('len', a.len())
        out('len2', a.len2())
        out('dist', a.dist(b))
        out('dist2', a.dist2(b))
        b.scaleToLen(4.)
        out('scaleToLen', b, b.len())
        out('scaledToLen', b.scaledToLen(2.), b.len(), b.scaledToLen(2.).len())
        out('normalized', b.normalized(), b.len(), b.normalized().len())
        b.normalize()
        out('normalize', b, b.len())
        out('cross', a, b, a.cross(b))

        out()
        out('segmentDist2', Vec3().segmentDist2(Vec3(1, 0, 0), Vec3(2, 0, 0)))
        out('triDist2', Vec3().triDist2(Vec3(1, 0, 0), Vec3(2, 0, 0), Vec3(2, 1, 0)))
        out('inTri', Vec3().inTri(Vec3(1, 0, 0), Vec3(2, 0, 0), Vec3(2, 1, 0)))
        out('inTri', Vec3().inTri(Vec3(-1, 0, 0), Vec3(2, -1, 0), Vec3(2, 1, 0)))
        out('angle', Vec3.angle(Vec3(-1, 0, 0), Vec3(2, -1, 0), Vec3(2, 1, 0)))
        out('angle', Vec3.angle(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0)))
        out('dihedralAngle', Vec3.dihedralAngle(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0), Vec3(0, 0, 1)))
        out('projToPlane', Vec3(0, 0).projToPlane(Vec3(2, -1, 0), Vec3(-1, 0, 0), Vec3(2, 1, 0)))
        out('projToPlane2', Vec3(0, 0).projToPlane2(Vec3(0, 0, -1), Vec3(0, 0, 1)))
        out('triArea2', Vec3.triArea2(Vec3(0, 0, 0), Vec3(0, 0, 1), Vec3(0, 1, 0)))



if __name__ == '__main__':
    fout = open('regressions/tmp.Vec3.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(Vec3TestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
