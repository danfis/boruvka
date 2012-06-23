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

#ifndef __BOR_TASK_POOL_H__
#define __BOR_TASK_POOL_H__

#include <pthread.h>
#include <semaphore.h>
#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Task Pool - Pool of Threads
 * ============================
 *
 * See bor_task_pool_t.
 */

struct _bor_task_pool_t;

/**
 * Info about thread.
 */
struct _bor_task_pool_thinfo_t {
    int id; /*!< ID of thread - is same as index in .threads[] member of
                 bor_task_pool_t struct */
};
typedef struct _bor_task_pool_thinfo_t bor_task_pool_thinfo_t;


struct _bor_task_pool_thread_t {
    bor_task_pool_thinfo_t info; /*!< Thread info */
    struct _bor_task_pool_t *task_pool; /*!< Back reference to main struct */
    pthread_t th;                /*!< POSIX thread */
    bor_list_t tasks;            /*!< List of pending tasks */

    pthread_mutex_t lock;        /*!< Global lock for thread synchronization */
    sem_t full, empty;           /*!< Full/Empty semaphores for pending tasks */

    int pending;                 /*!< Number of pending tasks */
    pthread_cond_t pending_cond; /*!< Conditional variable to allow user
                                      code to wait until all tasks are
                                      finished */
};
typedef struct _bor_task_pool_thread_t bor_task_pool_thread_t;


struct _bor_task_pool_t {
    bor_task_pool_thread_t **threads; /*!< Array of threads */
    size_t threads_len;               /*!< Number of .threads array */
    int started;                      /*!< Set to 1 if all threads were
                                           started */
};
typedef struct _bor_task_pool_t bor_task_pool_t;



/**
 * Callback used as task
 */
typedef void (*bor_task_pool_fn)(int id, void *data,
                                 const bor_task_pool_thinfo_t *thinfo);

/**
 * Creates task pool containing {num_threads} threads.
 * Note that borTaskPoolRun() must be called in order to actually start the
 * threads.
 */
bor_task_pool_t *borTaskPoolNew(size_t num_threads);

/**
 * Waits for all tasks to be processed and then deletes task pool.
 */
void borTaskPoolDel(bor_task_pool_t *t);

/**
 * Returns number of threads in task pool.
 */
_bor_inline size_t borTaskPoolSize(const bor_task_pool_t *t);

/**
 * Adds task to {tid}'th thread in pool.
 * Returns 0 on success.
 */
void borTaskPoolAdd(bor_task_pool_t *t, int tid,
                    bor_task_pool_fn fn, int id, void *data);

/**
 * Start all threads in pool.
 */
void borTaskPoolRun(bor_task_pool_t *t);

/**
 * Returns number of pending tasks of {id}'th thread.
 */
int borTaskPoolPending(bor_task_pool_t *t, int id);

/**
 * Blocks until all tasks of {id}'th thread aren't finished
 */
void borTaskPoolBarrier(bor_task_pool_t *t, int id);

/**** INLINES ****/
_bor_inline size_t borTaskPoolSize(const bor_task_pool_t *t)
{
    return t->threads_len;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_TASK_POOL_H__ */

