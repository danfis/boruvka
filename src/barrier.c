/***
 * fermat
 * -------
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

#include <fermat/barrier.h>
#include <fermat/alloc.h>

fer_barrier_t *ferBarrierNew(uint16_t goal)
{
    fer_barrier_t *b;

    b = FER_ALLOC(fer_barrier_t);
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->goal = goal;

    b->cur = 0;

    return b;
}

void ferBarrierDel(fer_barrier_t *b)
{
    pthread_mutex_destroy(&b->lock);
    pthread_cond_destroy(&b->cond);
    free(b);
}

void ferBarrier(fer_barrier_t *b)
{
    pthread_mutex_lock(&b->lock);
    ++b->cur;
    if (b->cur == b->goal){
        // unblock barrier
        pthread_cond_broadcast(&b->cond);
        b->cur = 0;
    }else{
        // lock on barrier
        pthread_cond_wait(&b->cond, &b->lock);
    }
    pthread_mutex_unlock(&b->lock);
}
