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

class QuatTestCase(unittest.TestCase):
    def testNumberProtocol(self):
        out('---- NumberProtocol ----')
        a = Quat()
        out(a)
        a = Quat(1)
        out(a)
        a = Quat(1, 2)
        out(a)
        a = Quat(1, 2, 3)
        out(a)
        a = Quat(1, 2, 3, 4)
        out(a)

        out()

        b = Quat(3.1, 1.2, 0.4, 11.2)
        out(str(a), '*', 1.5, '=', str(a * 1.5))
        out(1.5, '*', str(a), '=', str(1.5 * a))

        out(str(-a))

        out(str(a), '/', 2, '=', str(a / 2))

        out()
        b = Quat(3.1, 1.2, 0.4)
        b *= 2.
        out(str(b))
        b /= 2.
        out(str(b))

        out()

        a = Quat()
        v = Vec3(1., 1., 1.)
        a.setAngleAxis(math.pi, Vec3(1, 0, 0))
        out(a, a.inverted())
        out(a)
        out(v)
        a.rot(v)
        out(v)
        a.invert()
        out(a.roted(v))

        out()
        b = Quat().setAngleAxis(math.pi, Vec3(0, 1, 0))
        out(b)
        out(a.muled(b))
        out(a.muled(b).roted(v))

    def testSeqProtocol(self):
        out('---- SeqProtocol ----')
        a = Quat(1, 2, 3, 4)
        out(len(a), a[0], a[1], a[2], a[3])
        out(a.x, a.y, a.z, a.w)
        for i in a:
            out(i)

    def testCmp(self):
        self.assertTrue(Quat(1, 2, 3, 4) == Quat(1, 2, 3, 4))
        self.assertTrue(Quat(1, 2, 3, 4) != Quat(2, 2, 3, 4))

    def testMethods(self):
        out('---- Methods ----')
        a = Quat(1, 2, 3, 4)
        b = a.copy()
        b *= 2
        out(a, b)

        out('len', a.len())
        out('len2', a.len2())
        out('normalized', b.normalized(), b.len(), b.normalized().len())
        b.normalize()
        out('normalize', b, b.len())



if __name__ == '__main__':
    fout = open('regressions/tmp.Quat.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(QuatTestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
