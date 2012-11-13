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

#include <boruvka/alloc.h>
#include <boruvka/rand-mt-parallel.h>

static bor_rand_mt_parallel_t *prealloc(int threads, int size)
{
    bor_rand_mt_parallel_t *r;
    int i;

    r = BOR_ALLOC(bor_rand_mt_parallel_t);
    r->rand = NULL;
    pthread_mutex_init(&r->lock, NULL);

    r->threads = threads;
    r->size    = size;

    r->pool = BOR_ALLOC_ARR(double *, threads);
    for (i = 0; i < threads; i++){
        r->pool[i] = BOR_ALLOC_ARR(double, size);
    }

    r->next = BOR_ALLOC_ARR(int, threads);

    return r;
}

static void refillAll(bor_rand_mt_parallel_t *r)
{
    int i;

    for (i = 0; i < r->threads; i++){
        __borRandMTParallelRefill(r, i);
    }
}

bor_rand_mt_parallel_t *borRandMTParallelNew(int threads, int size,
                                             uint32_t seed)
{
    bor_rand_mt_parallel_t *r;

    r = prealloc(threads, size);
    r->rand = borRandMTNew(seed);
    refillAll(r);

    return r;
}

bor_rand_mt_parallel_t *borRandMTParallelNew2(int threads, int size,
                                              uint32_t *seed, uint32_t seedlen)
{
    bor_rand_mt_parallel_t *r;

    r = prealloc(threads, size);
    r->rand = borRandMTNew2(seed, seedlen);
    refillAll(r);

    return r;
}

bor_rand_mt_parallel_t *borRandMTParallelNewAuto(int threads, int size)
{
    bor_rand_mt_parallel_t *r;

    r = prealloc(threads, size);
    r->rand = borRandMTNewAuto();
    refillAll(r);

    return r;
}

void borRandMTParallelDel(bor_rand_mt_parallel_t *r)
{
    int i;

    borRandMTDel(r->rand);
    pthread_mutex_destroy(&r->lock);

    for (i = 0; i < r->threads; i++){
        BOR_FREE(r->pool[i]);
    }
    BOR_FREE(r->pool);
    BOR_FREE(r->next);

    BOR_FREE(r);
}

void __borRandMTParallelRefill(bor_rand_mt_parallel_t *r, int thread_id)
{
    int i;

    pthread_mutex_lock(&r->lock);
    for (i = 0; i < r->size; i++){
        r->pool[thread_id][i] = borRandMT01(r->rand);
    }
    r->next[thread_id] = 0;
    pthread_mutex_unlock(&r->lock);
}
