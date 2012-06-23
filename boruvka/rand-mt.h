/***
 * Boruvka
 * --------
 * Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
 * Based on code by Richard J. Wagner - v1.1
 *      (http://www-personal.umich.edu/~wagnerr/MersenneTwister.html)
 *
 * Copyright (c)1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * Copyright (c)2000 - 2009, Richard J. Wagner
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_RAND_MT_H__
#define __BOR_RAND_MT_H__

#include <stdint.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Length of state vector */
#define BOR_RAND_MT_N 624
/** Period */
#define BOR_RAND_MT_M 397

/**
 * RandMT - Mersenne Twister Random Number Generator
 * ==================================================
 *
 * The Mersenne Twister is an algorithm for generating random numbers.  It
 * was designed with consideration of the flaws in various other generators.
 * The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
 * are far greater.  The generator is also fast; it avoids multiplication and
 * division, and it benefits from caches and pipelines.  For more information
 * see the inventors' web page at
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 *
 * Reference:
 * M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
 * Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
 * Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.
 */
struct _bor_rand_mt_t {
    uint32_t state[BOR_RAND_MT_N]; /*!< Internal state */
    uint32_t *next;                /*!< Next value from state[] */
    int left;                      /*!< Number of values left before reload
                                        needed */
};
typedef struct _bor_rand_mt_t bor_rand_mt_t;

/**
 * Functions
 * ----------
 */

/**
 * Creates generator with given seed.
 */
bor_rand_mt_t *borRandMTNew(uint32_t seed);

/**
 * Creates generator with more seeds.
 */
bor_rand_mt_t *borRandMTNew2(uint32_t *seed, uint32_t seedlen);

/**
 * Similar to borRandMTNew() but seed is chosen automatically.
 */
bor_rand_mt_t *borRandMTNewAuto(void);

/**
 * Reseeds generator.
 */
void borRandMTReseed(bor_rand_mt_t *r, uint32_t seed);

/**
 * Similar to *Reseed() but more seeds are used.
 */
void borRandMTReseed2(bor_rand_mt_t *r, uint32_t *seed, uint32_t seedlen);

/**
 * Similar to *Reseed() function but seed is chosen automatically from
 * /dev/urandom if available or from time() and clock().
 */
void borRandMTReseedAuto(bor_rand_mt_t *r);

/**
 * Deletes generator.
 */
void borRandMTDel(bor_rand_mt_t *r);

/**
 * Returns randomly generated number in range [from, to).
 */
_bor_inline double borRandMT(bor_rand_mt_t *r, bor_real_t from, bor_real_t to);

/**
 * Returns number between [0-1) real interval.
 */
_bor_inline double borRandMT01(bor_rand_mt_t *r);

/**
 * Returns number between [0-1] real interval.
 */
_bor_inline double borRandMT01Closed(bor_rand_mt_t *r);

/**
 * Returns number between (0-1) real interval.
 */
_bor_inline double borRandMT01Open(bor_rand_mt_t *r);

/**
 * Returns number between [0-1) real interval with 53-bit resolution.
 */
_bor_inline double borRandMT01_53(bor_rand_mt_t *r);

/**
 * Returns random integer number in interval [0, 2^32-1].
 */
_bor_inline uint32_t borRandMTInt(bor_rand_mt_t *r);


/**
 * Returns number from a normal (Gaussian) distribution.
 */
_bor_inline double borRandMTNormal(bor_rand_mt_t *r, double mean, double stddev);


/**
 * Reloads generator.
 * This is for internal use.
 */
void __borRandMTReload(bor_rand_mt_t *r);

/**** INLINES ****/
_bor_inline double borRandMT01(bor_rand_mt_t *r)
{
    return (double)borRandMTInt(r) * (1.0/4294967296.0);
}

_bor_inline double borRandMT01Closed(bor_rand_mt_t *r)
{
    return (double)borRandMTInt(r) * (1.0/4294967295.0);
}

_bor_inline double borRandMT01Open(bor_rand_mt_t *r)
{
    return ((double)borRandMTInt(r) + 0.5) * (1.0/4294967296.0);
}

_bor_inline double borRandMT01_53(bor_rand_mt_t *r)
{
    uint32_t a = borRandMTInt(r) >> 5;
    uint32_t b = borRandMTInt(r) >> 6;
    return (a * 67108864.0 + b) * (1.0/9007199254740992.0);  /* by Isaku Wada */
}

_bor_inline double borRandMT(bor_rand_mt_t *r, bor_real_t from, bor_real_t to)
{
    double val;
    val  = borRandMT01(r);
    val *= to - from;
    val += from;
    return val;
}

_bor_inline uint32_t borRandMTInt(bor_rand_mt_t *r)
{
    /* Pull a 32-bit integer from the generator state */
    /* Every other access function simply transforms the numbers extracted here */
    register uint32_t s1;
    
    if (r->left == 0)
        __borRandMTReload(r);
    --r->left;
    
    s1 = *r->next++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9d2c5680UL;
    s1 ^= (s1 << 15) & 0xefc60000UL;
    return (s1 ^ (s1 >> 18));
}

_bor_inline double borRandMTNormal(bor_rand_mt_t *g, double mean, double stddev)
{
    /* Return a real number from a normal (Gaussian) distribution with given */
    /* mean and standard deviation by polar form of Box-Muller transformation */
    double x, y, r;
    do
    {
        x = 2.0 * borRandMT01(g) - 1.0;
        y = 2.0 * borRandMT01(g) - 1.0;
        r = x * x + y * y;
    }
    while ( r >= 1.0 || r == 0.0 );
    double s = sqrt( -2.0 * log(r) / r );
    return mean + x * s * stddev;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_RAND_MT_H__ */
