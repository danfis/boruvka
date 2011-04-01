from core cimport *
cimport rand
cimport rand_mt
cimport timer
cimport vec2


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


##
# Timer
##
cdef class Timer:
    cdef timer.fer_timer_t _timer
    def __cinit__(self):
        pass

    def start(self):
        timer.ferTimerStart(&self._timer)
    def stop(self):
        timer.ferTimerStop(&self._timer)

    def ns(self):
        return timer.ferTimerElapsedNs(&self._timer)
    def us(self):
        return timer.ferTimerElapsedUs(&self._timer)
    def ms(self):
        return timer.ferTimerElapsedMs(&self._timer)
    def s(self):
        return timer.ferTimerElapsedS(&self._timer)
    def m(self):
        return timer.ferTimerElapsedM(&self._timer)
    def h(self):
        return timer.ferTimerElapsedH(&self._timer)

    def inNs(self):
        return timer.ferTimerElapsedInNs(&self._timer)
    def inUs(self):
        return timer.ferTimerElapsedInUs(&self._timer)
    def inMs(self):
        return timer.ferTimerElapsedInMs(&self._timer)
    def inS(self):
        return timer.ferTimerElapsedInS(&self._timer)
    def inM(self):
        return timer.ferTimerElapsedInM(&self._timer)
    def inH(self):
        return timer.ferTimerElapsedInH(&self._timer)


##
# Vec2
##
cdef class Vec2:
    cdef vec2.fer_vec2_t _vec

    property x:
        def __get__(self):
            return vec2.ferVec2X(&self._vec)
        def __set__(self, fer_real_t val):
            vec2.ferVec2SetX(&self._vec, val)

    property y:
        def __get__(self):
            return vec2.ferVec2Y(&self._vec)
        def __set__(self, fer_real_t val):
            vec2.ferVec2SetY(&self._vec, val)

    def __cinit__(self, fer_real_t x = 0, fer_real_t y = 0):
        vec2.ferVec2Set(&self._vec, x, y)

    def __str__(self):
        return '<fermat.Vec2 ({0}, {1})>'.format(self.x, self.y)

    def dist2(self, Vec2 w):
        return vec2.ferVec2Dist2(&self._vec, &w._vec)
    def dist(self, Vec2 w):
        return vec2.ferVec2Dist(&self._vec, &w._vec)
    def len2(self):
        return vec2.ferVec2Len2(&self._vec)
    def len(self):
        return vec2.ferVec2Len(&self._vec)

    def add(self, Vec2 o):
        vec2.ferVec2Add(&self._vec, &o._vec)
        return self
    def add2(self, Vec2 o):
        cdef Vec2 w = Vec2()
        vec2.ferVec2Add2(&w._vec, &self._vec, &o._vec)
        return w
    def sub(self, Vec2 o):
        vec2.ferVec2Sub(&self._vec, &o._vec)
        return self
    def sub2(self, Vec2 o):
        cdef Vec2 w = Vec2()
        vec2.ferVec2Sub2(&w._vec, &self._vec, &o._vec)
        return w

    def addConst(self, fer_real_t r):
        vec2.ferVec2AddConst(&self._vec, r)
        return self
    def addConst2(self, fer_real_t r):
        cdef Vec2 w
        vec2.ferVec2AddConst2(&w._vec, &self._vec, r)
        return w
    def subConst(self, fer_real_t r):
        vec2.ferVec2SubConst(&self._vec, r)
        return self
    def subConst2(self, fer_real_t r):
        cdef Vec2 w
        vec2.ferVec2SubConst2(&w._vec, &self._vec, r)
        return w

    def scaleToLen(self, fer_real_t l):
        vec2.ferVec2ScaleToLen(&self._vec, l)
        return self
    def normalize(self):
        vec2.ferVec2Normalize(&self._vec)
        return self

    def dot(self, Vec2 o):
        return vec2.ferVec2Dot(&self._vec, &o._vec)
    def mulComp(self, Vec2 o):
        vec2.ferVec2MulComp(&self._vec, &o._vec)
        return self
    def mulComp2(self, Vec2 o):
        cdef Vec2 w = Vec2()
        vec2.ferVec2MulComp2(&w._vec, &self._vec, &o._vec)
        return w

    def scale(self, fer_real_t f):
        vec2.ferVec2Scale(&self._vec, f)
        return self

    def area2(self, Vec2 b, Vec2 c):
        return vec2.ferVec2Area2(&self._vec, &b._vec, &c._vec)
    def angle(self, Vec2 b, Vec2 c):
        return vec2.ferVec2Angle(&self._vec, &b._vec, &c._vec)
    def signedAngle(self, Vec2 b, Vec2 c):
        return vec2.ferVec2SignedAngle(&self._vec, &b._vec, &c._vec)

    def projectOntoSegment(self, Vec2 a, Vec2 b):
        cdef Vec2 x
        cdef int res
        res = vec2.ferVec2ProjectionPointOntoSegment(&a._vec, &b._vec, &self._vec, &x._vec)
        if res == 0:
            return x
        return None
