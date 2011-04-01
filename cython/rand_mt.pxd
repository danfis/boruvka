from core cimport *

cdef extern from "../fermat/rand-mt.h":
    ctypedef float fer_rand_mt_t
    ctypedef int uint32_t

    fer_rand_mt_t *ferRandMTNew(uint32_t seed)
    fer_rand_mt_t *ferRandMTNew2(uint32_t *seed, uint32_t seedlen)
    fer_rand_mt_t *ferRandMTNewAuto()
    void ferRandMTReseed(fer_rand_mt_t *r, uint32_t seed)
    void ferRandMTReseed2(fer_rand_mt_t *r, uint32_t *seed, uint32_t seedlen)
    void ferRandMTReseedAuto(fer_rand_mt_t *r)
    void ferRandMTDel(fer_rand_mt_t *r)
    double ferRandMT(fer_rand_mt_t *r, fer_real_t f, fer_real_t t)
    double ferRandMT01(fer_rand_mt_t *r)
    double ferRandMT01Closed(fer_rand_mt_t *r)
    double ferRandMT01Open(fer_rand_mt_t *r)
    double ferRandMT01_53(fer_rand_mt_t *r)
    uint32_t ferRandMTInt(fer_rand_mt_t *r)
    double ferRandMTNormal(fer_rand_mt_t *r, double mean, double stddev)

