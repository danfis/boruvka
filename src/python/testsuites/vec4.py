import unittest
from fermat import *

fout = None
def out(*a):
    s = ''
    for i in a:
        s += str(i) + ' '
    fout.write(s)
    fout.write('\n')

class Vec4TestCase(unittest.TestCase):
    def testNumberProtocol(self):
        out('---- NumberProtocol ----')
        a = Vec4()
        out(a)
        a = Vec4(1)
        out(a)
        a = Vec4(1, 2)
        out(a)
        a = Vec4(1, 2, 3)
        out(a)
        a = Vec4(1, 2, 3, 4)
        out(a)

        out()

        b = Vec4(3.1, 1.2, 0.4, 11.2)
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
        b = Vec4(3.1, 1.2, 0.4)
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
        a = Vec4(1, 2, 3, 4)
        out(len(a), a[0], a[1], a[2], a[3])
        out(a.x, a.y, a.z, a.w)
        for i in a:
            out(i)

    def testCmp(self):
        self.assertTrue(Vec4(1, 2, 3, 4) == Vec4(1, 2, 3, 4))
        self.assertTrue(Vec4(1, 2, 3, 4) != Vec4(2, 2, 3, 4))

    def testMethods(self):
        out('---- Methods ----')
        a = Vec4(1, 2, 3)
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



if __name__ == '__main__':
    fout = open('regressions/tmp.Vec4.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(Vec4TestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
