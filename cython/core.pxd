
ctypedef float fer_real_t

cdef extern from "../fermat/core.h":
    fer_real_t FER_EPS
    fer_real_t FER_REAL_MAX
    fer_real_t FER_REAL_MIN
    fer_real_t FER_ONE
    fer_real_t FER_ZERO

    int ferSign(fer_real_t)
    int ferIsZero(fer_real_t val)
    int ferEq(fer_real_t a, fer_real_t b)
    int ferNEq(fer_real_t a, fer_real_t b)
    fer_real_t ferRecp(fer_real_t v)
    fer_real_t ferRsqrt(fer_real_t v)

