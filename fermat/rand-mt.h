/***
 * fermat
 * -------
 * Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
 * Based on code by Richard J. Wagner - v1.1
 *      (http://www-personal.umich.edu/~wagnerr/MersenneTwister.html)
 *
 * Copyright (c)1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * Copyright (c)2000 - 2009, Richard J. Wagner
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_RAND_MT_H__
#define __FER_RAND_MT_H__

#include <stdint.h>
#include <fermat/core.h>

/** Length of state vector */
#define FER_RAND_MT_N 624
/** Period */
#define FER_RAND_MT_M 397

/**
 * RandMT - Mersenne Twister
 * ==========================
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
struct _fer_rand_mt_t {
    uint32_t state[FER_RAND_MT_N]; /*!< Internal state */
    uint32_t *next;                /*!< Next value from state[] */
    int left;                      /*!< Number of values left before reload
                                        needed */
};
typedef struct _fer_rand_mt_t fer_rand_mt_t;

/**
 * Functions
 * ----------
 */

/**
 * Creates generator with given seed.
 */
fer_rand_mt_t *ferRandMTNew(uint32_t seed);

/**
 * Creates generator with more seeds.
 */
fer_rand_mt_t *ferRandMTNew2(uint32_t *seed, uint32_t seedlen);

/**
 * Similar to ferRandMTNew() but seed is chosen automatically.
 */
fer_rand_mt_t *ferRandMTNewAuto(void);

/**
 * Reseeds generator.
 */
void ferRandMTReseed(fer_rand_mt_t *r, uint32_t seed);

/**
 * Similar to *Reseed() but more seeds are used.
 */
void ferRandMTReseed2(fer_rand_mt_t *r, uint32_t *seed, uint32_t seedlen);

/**
 * Similar to *Reseed() function but seed is chosen automatically from
 * /dev/urandom if available or from time() and clock().
 */
void ferRandMTReseedAuto(fer_rand_mt_t *r);

/**
 * Deletes generator.
 */
void ferRandMTDel(fer_rand_mt_t *r);

/**
 * Returns randomly generated number in range [from, to).
 */
_fer_inline double ferRandMT(fer_rand_mt_t *r, fer_real_t from, fer_real_t to);

/**
 * Returns number between [0-1) real interval.
 */
_fer_inline double ferRandMT01(fer_rand_mt_t *r);

/**
 * Returns number between [0-1] real interval.
 */
_fer_inline double ferRandMT01Closed(fer_rand_mt_t *r);

/**
 * Returns number between (0-1) real interval.
 */
_fer_inline double ferRandMT01Open(fer_rand_mt_t *r);

/**
 * Returns number between [0-1) real interval with 53-bit resolution.
 */
_fer_inline double ferRandMT01_53(fer_rand_mt_t *r);

/**
 * Returns random integer number in interval [0, 2^32-1].
 */
_fer_inline uint32_t ferRandMTInt(fer_rand_mt_t *r);


/**
 * Returns number from a normal (Gaussian) distribution.
 */
_fer_inline double ferRandMTNormal(fer_rand_mt_t *r, double mean, double stddev);


/**
 * Reloads generator.
 * This is for internal use.
 */
void __ferRandMTReload(fer_rand_mt_t *r);

/**** INLINES ****/
_fer_inline double ferRandMT01(fer_rand_mt_t *r)
{
    return (double)ferRandMTInt(r) * (1.0/4294967296.0);
}

_fer_inline double ferRandMT01Closed(fer_rand_mt_t *r)
{
    return (double)ferRandMTInt(r) * (1.0/4294967295.0);
}

_fer_inline double ferRandMT01Open(fer_rand_mt_t *r)
{
    return ((double)ferRandMTInt(r) + 0.5) * (1.0/4294967296.0);
}

_fer_inline double ferRandMT01_53(fer_rand_mt_t *r)
{
    uint32_t a = ferRandMTInt(r) >> 5;
    uint32_t b = ferRandMTInt(r) >> 6;
    return (a * 67108864.0 + b) * (1.0/9007199254740992.0);  // by Isaku Wada
}

_fer_inline double ferRandMT(fer_rand_mt_t *r, fer_real_t from, fer_real_t to)
{
    double val;
    val  = ferRandMT01(r);
    val *= to - from;
    val += from;
    return val;
}

_fer_inline uint32_t ferRandMTInt(fer_rand_mt_t *r)
{
    /* Pull a 32-bit integer from the generator state */
    /* Every other access function simply transforms the numbers extracted here */
    register uint32_t s1;
    
    if (r->left == 0)
        __ferRandMTReload(r);
    --r->left;
    
    s1 = *r->next++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9d2c5680UL;
    s1 ^= (s1 << 15) & 0xefc60000UL;
    return (s1 ^ (s1 >> 18));
}

_fer_inline double ferRandMTNormal(fer_rand_mt_t *g, double mean, double stddev)
{
    // Return a real number from a normal (Gaussian) distribution with given
    // mean and standard deviation by polar form of Box-Muller transformation
    double x, y, r;
    do
    {
        x = 2.0 * ferRandMT01(g) - 1.0;
        y = 2.0 * ferRandMT01(g) - 1.0;
        r = x * x + y * y;
    }
    while ( r >= 1.0 || r == 0.0 );
    double s = sqrt( -2.0 * log(r) / r );
    return mean + x * s * stddev;
}

#endif /* __FER_RAND_MT_H__ */
