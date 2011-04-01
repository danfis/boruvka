from core cimport *


cdef extern from "../fermat/rand.h":
    ctypedef float fer_rand_t

    void ferRandInit(fer_rand_t *)
    fer_real_t ferRand(fer_rand_t *r, fer_real_t f, fer_real_t t)
