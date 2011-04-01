from core cimport *
cimport rand
cimport rand_mt


# constants
EPS      = FER_EPS
REAL_MAX = FER_REAL_MAX
REAL_MIN = FER_REAL_MIN
ONE      = FER_ONE
ZERO     = FER_ZERO

# functions:
cpdef int sign(fer_real_t val):
    return ferSign(val)

cpdef int isZero(fer_real_t val):
    return ferIsZero(val)

cpdef int eq(fer_real_t a, fer_real_t b):
    return ferEq(a, b)
cpdef int neq(fer_real_t a, fer_real_t b):
    return ferNEq(a, b)

cpdef fer_real_t recp(fer_real_t v):
    return ferRecp(v)

cpdef fer_real_t rsqrt(fer_real_t v):
    return ferRsqrt(v)

cpdef swap(a, b):
    return (b, a)


##
# Rand
##
cdef class Rand:
    cdef rand.fer_rand_t _rand
    def __cinit__(self):
        rand.ferRandInit(&self._rand)

    def uniform(self, fer_real_t f, fer_real_t t):
        return rand.ferRand(&self._rand, f, t)


##
# RandMT
##
cdef class RandMT:
    cdef rand_mt.fer_rand_mt_t *_rand
    def __cinit__(self, seed = None):
        if seed is None:
            self._rand = rand_mt.ferRandMTNewAuto()
        else:
            self._rand = rand_mt.ferRandMTNew(seed)

    def __dealloc__(self):
        if self._rand:
            rand_mt.ferRandMTDel(self._rand)

    def reseed(self, seed = None):
        if seed is not None:
            rand_mt.ferRandMTReseed(self._rand, seed)
        else:
            rand_mt.ferRandMTReseedAuto(self._rand)


    def uniform(self, fer_real_t f, fer_real_t t):
        return rand_mt.ferRandMT(self._rand, f, t)

    def normal(self, fer_real_t mean, fer_real_t stddev):
        return rand_mt.ferRandMTNormal(self._rand, mean, stddev)
