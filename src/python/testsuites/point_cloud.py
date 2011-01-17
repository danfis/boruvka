import unittest
from common import *
from fermat import *

class TestCase(TC):
    def __init__(self, mn):
        TC.__init__(self, mn, 'PC')

    def testSeqProtocol(self):
        self.out('---- SeqProtocol ----')
        pc = PC()
        self.out(pc)
        pc.add(Vec3(1, 2, 3))
        self.out(pc)
        for p in pc:
            self.out(p)

        self.out()
        pc.addFromFile('../../testsuites/cube.txt')
        self.out(pc)
        #for p in pc:
        #    self.out(p)

        pc.permutate()
        self.out(pc)


    def testMethods(self):
        self.out('---- Methods ----')

