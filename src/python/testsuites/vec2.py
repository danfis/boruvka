import unittest
from fermat import *

fout = None
def out(*a):
    s = ''
    for i in a:
        s += str(i) + ' '
    fout.write(s)
    fout.write('\n')

class Vec2TestCase(unittest.TestCase):
    def testNumberProtocol(self):
        out('---- NumberProtocol ----')
        a = Vec2()
        out(str(a))
        a = Vec2(1)
        out(str(a))
        a = Vec2(1, 2)
        out(str(a))

        out()

        b = Vec2(3.1, 1.2)
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
        b = Vec2(3.1, 1.2)
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
        a = Vec2(1, 2)
        out(len(a), a[0], a[1])
        out(a.x, a.y)
        for i in a:
            out(i)

    def testCmp(self):
        self.assertTrue(Vec2(1, 2) == Vec2(1, 2))
        self.assertTrue(Vec2(1, 2) != Vec2(2, 2))

    def testMethods(self):
        out('---- Methods ----')
        a = Vec2(1, 2)
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

        out()
        out('area2', Vec2.area2(Vec2(0., 0.), Vec2(1., 0.), Vec2(0., 2.)))
        out('angle', Vec2.angle(Vec2(0., 0.), Vec2(1., 0.), Vec2(0., 2.)))
        out('angle', Vec2.angle(Vec2(1., 0.), Vec2(0., 0.), Vec2(0., 2.)))
        out('angle', Vec2.angle(Vec2(0., 2.), Vec2(0., 0.), Vec2(1., 0.)))
        out('signedAngle', Vec2.signedAngle(Vec2(1., 0.), Vec2(0., 0.), Vec2(0., 2.)))
        out('signedAngle', Vec2.signedAngle(Vec2(0., 2.), Vec2(0., 0.), Vec2(1., 0.)))

        out()
        p = Vec2(1, 1).projectionOntoSegment(Vec2(0, 0), Vec2(3, 0))
        out('projectionOntoSegment', p)
        p = Vec2(4, 1).projectionOntoSegment(Vec2(0, 0), Vec2(3, 0))
        out('projectionOntoSegment', p)
        out('inCircle', Vec2(1, 1).inCircle(Vec2(0, 0), Vec2(1, 0), Vec2(0, 2)))
        out('inCircle', Vec2(1, 1).inCircle(Vec2(0, 0), Vec2(1, 0), Vec2(0, 0.1)))
        out('liesOn', Vec2(1, 1).liesOn(Vec2(0, 0), Vec2(3, 3)))
        out('liesOn', Vec2(1, 1).liesOn(Vec2(0, 0), Vec2(3, 1)))
        out('collinear', Vec2(3, 3).collinear(Vec2(0, 0), Vec2(1, 1)))
        out('collinear', Vec2(3, 3).collinear(Vec2(0, 0), Vec2(1, 9)))
        out('inCone', Vec2(0.5, 0.5).inCone(Vec2(0, 2), Vec2(0, 0), Vec2(2, 0)))
        out('inCone', Vec2(0.5, 0.5).inCone(Vec2(0, 2), Vec2(10, 0), Vec2(2, 0)))
        out('intersectProp', Vec2.intersectProp(Vec2(0., 0.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('intersectProp', Vec2.intersectProp(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('intersect', Vec2.intersect(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('intersectPoint', Vec2.intersectPoint(Vec2(0., 0.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('intersectPoint', Vec2.intersectPoint(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('intersectPoint', Vec2.intersectPoint(Vec2(0., 2.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        out('onLeft', Vec2(1, 1).onLeft(Vec2(0, 0), Vec2(1, 0)))
        out('onLeft', Vec2(1, -1).onLeft(Vec2(0, 0), Vec2(1, 0)))
        out('segmentInRect', Vec2.segmentInRect(Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1), Vec2(-1, -1), Vec2(2, 2)))


if __name__ == '__main__':
    fout = open('regressions/tmp.Vec2.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(Vec2TestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
