import unittest

files = []

class TC(unittest.TestCase):
    def __init__(self, mn = 'runTest', name = ''):
        super(TC, self).__init__(mn)
        self.name = name

    def setUp(self):
        fn = 'regressions/tmp.{0}.out'.format(self.name)
        if fn in files:
            self.fout = open(fn, 'a')
        else:
            self.fout = open(fn, 'w')
            files.append(fn)

    def tearDown(self):
        self.fout.close()

    def out(self, *a):
        s = ''
        for i in a:
            s += str(i) + ' '
        self.fout.write(s)
        self.fout.write('\n')
        self.fout.flush()
