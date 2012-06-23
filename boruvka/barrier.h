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

#ifndef __FER_BARRIER_H__
#define __FER_BARRIER_H__

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
struct _fer_barrier_t {
    pthread_mutex_t lock; /*!< Lock for .cond */
    pthread_cond_t cond;  /*!< Conditional variable */
    uint16_t goal;        /*!< Goal value */
    uint16_t cur;         /*!< Current value of barrier */
};
typedef struct _fer_barrier_t fer_barrier_t;

/**
 * Creates new barrier
 */
fer_barrier_t *ferBarrierNew(uint16_t goal);

/**
 * Deletes barrier
 */
void ferBarrierDel(fer_barrier_t *b);

/**
 * Block on barrier
 */
void ferBarrier(fer_barrier_t *b);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_BARRIER_H__ */


