/***
 * Boruvka
 * --------
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

#ifndef __BOR_BARRIER_H__
#define __BOR_BARRIER_H__

#include <pthread.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Barrier Synchronization
 * ========================
 *
 */
struct _bor_barrier_t {
    pthread_mutex_t lock; /*!< Lock for .cond */
    pthread_cond_t cond;  /*!< Conditional variable */
    uint16_t goal;        /*!< Goal value */
    uint16_t cur;         /*!< Current value of barrier */
};
typedef struct _bor_barrier_t bor_barrier_t;

/**
 * Creates new barrier
 */
bor_barrier_t *borBarrierNew(uint16_t goal);

/**
 * Deletes barrier
 */
void borBarrierDel(bor_barrier_t *b);

/**
 * Block on barrier
 */
void borBarrier(bor_barrier_t *b);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_BARRIER_H__ */


