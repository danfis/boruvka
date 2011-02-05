import unittest
from common import *
from fermat import *

class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'Vec2')

    def testNumberProtocol(self):
        self.out('---- NumberProtocol ----')
        a = Vec2()
        self.out(str(a))
        a = Vec2(1)
        self.out(str(a))
        a = Vec2(1, 2)
        self.out(str(a))

        self.out()

        b = Vec2(3.1, 1.2)
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
        b = Vec2(3.1, 1.2)
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
        a = Vec2(1, 2)
        self.out(len(a), a[0], a[1])
        self.out(a.x, a.y)
        for i in a:
            self.out(i)

    def testCmp(self):
        self.assertTrue(Vec2(1, 2) == Vec2(1, 2))
        self.assertTrue(Vec2(1, 2) != Vec2(2, 2))

    def testMethods(self):
        self.out('---- Methods ----')
        a = Vec2(1, 2)
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

        self.out()
        self.out('area2', Vec2.area2(Vec2(0., 0.), Vec2(1., 0.), Vec2(0., 2.)))
        self.out('angle', Vec2.angle(Vec2(0., 0.), Vec2(1., 0.), Vec2(0., 2.)))
        self.out('angle', Vec2.angle(Vec2(1., 0.), Vec2(0., 0.), Vec2(0., 2.)))
        self.out('angle', Vec2.angle(Vec2(0., 2.), Vec2(0., 0.), Vec2(1., 0.)))
        self.out('signedAngle', Vec2.signedAngle(Vec2(1., 0.), Vec2(0., 0.), Vec2(0., 2.)))
        self.out('signedAngle', Vec2.signedAngle(Vec2(0., 2.), Vec2(0., 0.), Vec2(1., 0.)))

        self.out()
        p = Vec2(1, 1).projectionOntoSegment(Vec2(0, 0), Vec2(3, 0))
        self.out('projectionOntoSegment', p)
        p = Vec2(4, 1).projectionOntoSegment(Vec2(0, 0), Vec2(3, 0))
        self.out('projectionOntoSegment', p)
        self.out('inCircle', Vec2(1, 1).inCircle(Vec2(0, 0), Vec2(1, 0), Vec2(0, 2)))
        self.out('inCircle', Vec2(1, 1).inCircle(Vec2(0, 0), Vec2(1, 0), Vec2(0, 0.1)))
        self.out('liesOn', Vec2(1, 1).liesOn(Vec2(0, 0), Vec2(3, 3)))
        self.out('liesOn', Vec2(1, 1).liesOn(Vec2(0, 0), Vec2(3, 1)))
        self.out('collinear', Vec2(3, 3).collinear(Vec2(0, 0), Vec2(1, 1)))
        self.out('collinear', Vec2(3, 3).collinear(Vec2(0, 0), Vec2(1, 9)))
        self.out('inCone', Vec2(0.5, 0.5).inCone(Vec2(0, 2), Vec2(0, 0), Vec2(2, 0)))
        self.out('inCone', Vec2(0.5, 0.5).inCone(Vec2(0, 2), Vec2(10, 0), Vec2(2, 0)))
        self.out('intersectProp', Vec2.intersectProp(Vec2(0., 0.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('intersectProp', Vec2.intersectProp(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('intersect', Vec2.intersect(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('intersectPoint', Vec2.intersectPoint(Vec2(0., 0.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('intersectPoint', Vec2.intersectPoint(Vec2(0., 1.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('intersectPoint', Vec2.intersectPoint(Vec2(0., 2.), Vec2(1, 1), Vec2(0, 1), Vec2(1, 0)))
        self.out('onLeft', Vec2(1, 1).onLeft(Vec2(0, 0), Vec2(1, 0)))
        self.out('onLeft', Vec2(1, -1).onLeft(Vec2(0, 0), Vec2(1, 0)))
        self.out('segmentInRect', Vec2.segmentInRect(Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1), Vec2(-1, -1), Vec2(2, 2)))
