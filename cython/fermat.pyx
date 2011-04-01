from core cimport *


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


def say_hello_to(name):
    print("Hello {0}!".format(name))
