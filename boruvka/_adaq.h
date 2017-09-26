/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_ADAQ_H__
#define __BOR_ADAQ_H__

#include <boruvka/compiler.h>
#include <boruvka/pairheap.h>
#include <boruvka/_bucketq.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Adaptive priority queue that can switch between bucket queue and
 * pairheap queue.
 */
struct bor_adaq {
    bor_bucketq_t bucket;
    bor_pairheap_t *heap;
    int size;
};
typedef struct bor_adaq bor_adaq_t;

/**
 * Initializes priority queue.
 */
void borAdaQInit(bor_adaq_t *pq);

/**
 * Frees allocated resources.
 */
void borAdaQFree(bor_adaq_t *pq);

/**
 * Inserts an element into queue.
 * If the key >= BOR_ADA_SIZE the program terminates.
 */
void borAdaQPush(bor_adaq_t *pq, int key, TYPE value);

/**
 * Removes and returns the lowest element.
 * If the queue is empty the program terminates.
 */
TYPE borAdaQPop(bor_adaq_t *pq, int *key);

/**
 * Returns true if the queue is empty.
 */
_bor_inline int borAdaQIsEmpty(const bor_adaq_t *pq);

/**** INLINES ****/
_bor_inline int borAdaQIsEmpty(const bor_adaq_t *pq)
{
    return pq->size == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_ADAQ_H__ */

