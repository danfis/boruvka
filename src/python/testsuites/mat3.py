import unittest
import math
from fermat import *

fout = None
def out(*a):
    s = ''
    for i in a:
        s += str(i) + ' '
    fout.write(s)
    fout.write('\n')

class Mat3TestCase(unittest.TestCase):
    def testNumberProtocol(self):
        out('---- NumberProtocol ----')
        a = Mat3()
        out(a)
        a = Mat3(1)
        out(a)
        a = Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9)
        out(a)

        v2 = Vec2(2, 3)
        v3 = Vec3(1, 2, 3)
        out()
        b = Mat3(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3)
        out(str(a), '+', 1.5, '=', str(a + 1.5))
        out(1.5, '+', str(a), '=', str(1.5 + a))
        out(str(a), '-', 1.5, '=', str(a - 1.5))
        out(str(a), '*', 1.5, '=', str(a * 1.5))
        out(1.5, '*', str(a), '=', str(1.5 * a))
        out(a, '*', b, '=', a * b)
        out(a, '*', v2, '=', a * v2)
        out(a, '*', v3, '=', a * v3)

        out(-a)

        out(a, '/', 2, '=', a / 2)

        out()
        b = Mat3(3.1, 1.2, 0.4, 1.2, 4.3, 1.1, 6.5, 1, 3)
        out(b)
        b *= 2.
        out(b)
        b /= 2.
        out(b)
        b += 2.
        out(b)
        b -= 2.
        out(b)

    def testSeqProtocol(self):
        out('---- SeqProtocol ----')
        a = Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9)
        out(len(a), a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8])
        for i in a:
            out(i)

    def testMethods(self):
        out('---- Methods ----')
        a = Mat3()
        a.setIdentity()
        out(a)
        a.setZero()
        out(a)
        a.setScale(1.1)
        out(a)
        a.setTranslate(Vec2(1, 2))
        out(a)
        a.setRot(math.pi / 3)
        out(a)

        a.trScale(3.)
        out(a)
        a.translate(Vec2(1, 2))
        out(a)
        a.rot(-math.pi / 3)

        out()
        b = Mat3()
        b.setTranslate(Vec2(4, 3))
        out(a.composed(b))
        a.compose(b)
        out(a)

        out(a.muledComp(b))
        a.mulComp(b)
        out(a)

        out(a.transposed())
        a.transpose()
        out(a)

        out(a.regular())
        out(a.singular())
        out(a.det())
        out(a.inved())
        a.inv()
        out(a)

        out()
        out(a)
        c = a.copy()
        c += 10
        out(a)
        out(c)




if __name__ == '__main__':
    fout = open('regressions/tmp.Mat3.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(Mat3TestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
