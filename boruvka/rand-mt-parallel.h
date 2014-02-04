/***
 * Boruvka
 * --------
 * Copyright (c)2012 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_RAND_MT_PARALLEL_H__
#define __BOR_RAND_MT_PARALLEL_H__

#include <pthread.h>
#include <boruvka/rand-mt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Mersenne Twister Random Number Generator for Parallel Use
 * ==========================================================
 *
 * This is random number generator implemented so that parallel threads can
 * access to a single stream.
 * For each thread is allocated array of pre-generated numbers that each
 * thread can read without blocking and once a re-fill is needed the
 * mersenne twister RNG is locked by mutex. Although it is not the same as
 * have one stream of randomly generated numbers it is blindly assumed that
 * this implementation is close enough.
 */
struct _bor_rand_mt_parallel_t {
    bor_rand_mt_t *rand;  /*!< Random number generator */
    pthread_mutex_t lock; /*!< Lock for exclusive access to .rand */
    int threads;          /*!< Number of concurent threads */
    int size;             /*!< Size of pre-generated array of numbers */
    double **pool;        /*!< Pool of pre-generated arrays */
    int *next;            /*!< Array of pointers to next number in array
                               for each thread */
};
typedef struct _bor_rand_mt_parallel_t bor_rand_mt_parallel_t;

/**
 * Allocates a new parallel RNG.
 * {threads} is number of concurent threads that will access the RNG.
 * {size} is a number of pre-generated numbers for each thread.
 * The rest of parameters are same as in borRandMTNew*() functions.
 */
bor_rand_mt_parallel_t *borRandMTParallelNew(int threads, int size,
                                             uint32_t seed);
bor_rand_mt_parallel_t *borRandMTParallelNew2(int threads, int size,
                                              uint32_t *seed, uint32_t seedlen);
bor_rand_mt_parallel_t *borRandMTParallelNewAuto(int threads, int size);

/**
 * Frees all allocated resources.
 */
void borRandMTParallelDel(bor_rand_mt_parallel_t *r);

/**
 * Returns randomly generated number in range [from, to).
 * {thread_id} is ID of the thread counted from 0 up to {threads}-1 as
 * specified in borRandMTParallelNew*() function.
 */
_bor_inline double borRandMTParallel(bor_rand_mt_parallel_t *r, int thread_id,
                                     bor_real_t from, bor_real_t to);

/**
 * Returns number between [0-1) real interval.
 */
_bor_inline double borRandMTParallel01(bor_rand_mt_parallel_t *r, int thread_id);


/**
 * Returns number from a normal (Gaussian) distribution.
 */
_bor_inline double borRandMTParallelNormal(bor_rand_mt_parallel_t *r, int thid,
                                           double mean, double stddev);

/** Refills a thread's pool.
 *  For internal use only. */
void __borRandMTParallelRefill(bor_rand_mt_parallel_t *r, int thread_id);

/**** INLINES ****/
_bor_inline double borRandMTParallel(bor_rand_mt_parallel_t *r, int thread_id,
                                     bor_real_t from, bor_real_t to)
{
    double val;
    val = borRandMTParallel01(r, thread_id);
    return (val * (to - from)) + from;
}

_bor_inline double borRandMTParallel01(bor_rand_mt_parallel_t *r, int thread_id)
{
    if (r->next[thread_id] == r->size)
        __borRandMTParallelRefill(r, thread_id);
    return r->pool[thread_id][r->next[thread_id]++];
}

_bor_inline double borRandMTParallelNormal(bor_rand_mt_parallel_t *g, int thid,
                                           double mean, double stddev)
{
    /* Return a real number from a normal (Gaussian) distribution with given */
    /* mean and standard deviation by polar form of Box-Muller transformation */
    double x, y, r;
    do
    {
        x = 2.0 * borRandMTParallel01(g, thid) - 1.0;
        y = 2.0 * borRandMTParallel01(g, thid) - 1.0;
        r = x * x + y * y;
    }
    while ( r >= 1.0 || r == 0.0 );
    double s = sqrt( -2.0 * log(r) / r );
    return mean + x * s * stddev;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_RAND_MT_PARALLEL_H__ */
