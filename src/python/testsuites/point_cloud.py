import unittest
from fermat import *

fout = None
def out(*a):
    s = ''
    for i in a:
        s += str(i) + ' '
    fout.write(s)
    fout.write('\n')

class PCTestCase(unittest.TestCase):
    def testSeqProtocol(self):
        out('---- SeqProtocol ----')
        pc = PC()
        out(pc)
        pc.add(Vec3(1, 2, 3))
        out(pc)
        for p in pc:
            out(p)

        out()
        pc.addFromFile('../../testsuites/cube.txt')
        out(pc)
        #for p in pc:
        #    out(p)

        pc.permutate()
        out(pc)


    def testMethods(self):
        out('---- Methods ----')



if __name__ == '__main__':
    fout = open('regressions/tmp.PC.out', 'w')
    suite = unittest.TestLoader().loadTestsFromTestCase(PCTestCase)
    unittest.TextTestRunner(verbosity=2).run(suite)
    fout.close()
