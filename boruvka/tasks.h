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

#ifndef __BOR_TASKS_H__
#define __BOR_TASKS_H__

#include <pthread.h>
#include <semaphore.h>
#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Tasks - Abstraction for Paralelization Using Threads
 * =====================================================
 */
struct _bor_tasks_t {
    bor_list_t tasks;     /*!< List of tasks */
    bor_list_t threads;   /*!< List of all threads */
    size_t threads_len;   /*!< Number of threads in list */
    int next_id;          /*!< Next unique ID for threads */

    pthread_mutex_t lock; /*!< Global lock for thread synchronization */
    sem_t full, empty;    /*!< Full/Empty semaphores for tasks queue */

    int pending;                  /*!< Number of pending tasks */
    pthread_cond_t pending_cond;  /*!< Conditional variable to allow user
                                       code to wait until all tasks are
                                       finished */
};
typedef struct _bor_tasks_t bor_tasks_t;

/**
 * Info about thread.
 */
struct _bor_tasks_thinfo_t {
    int id; /*!< Unique ID of thread */
};
typedef struct _bor_tasks_thinfo_t bor_tasks_thinfo_t;


/**
 * Callback used as task
 */
typedef void (*bor_tasks_fn)(int id, void *data,
                             const bor_tasks_thinfo_t *thinfo);

/**
 * Creates empty task queue with {num_threads} threads waiting.
 */
bor_tasks_t *borTasksNew(size_t num_threads);

/**
 * Waits for all tasks to be processed and then deletes task queue.
 * This is blocking call.
 *
 * Note that borTasksRun() must be called before this!
 */
void borTasksDel(bor_tasks_t *t);

/**
 * Returns number of threads.
 */
_bor_inline size_t borTasksNumThreads(const bor_tasks_t *t);

/**
 * Returns number of threads.
 */
_bor_inline size_t borTasksSize(const bor_tasks_t *t);

/**
 * Waits for current tasks to be processed, empty task queue and then
 * deletes the queue.
 *
 * Note that borTasksRun() must be called before this!
 */
void borTasksCancelDel(bor_tasks_t *t);

/**
 * Adds task to an end of queue.
 * Returns 0 on success.
 */
void borTasksAdd(bor_tasks_t *t, bor_tasks_fn fn, int id, void *data);

/**
 * Run tasks
 */
void borTasksRun(bor_tasks_t *t);

/**
 * Returns number of pending tasks
 */
int borTasksPending(bor_tasks_t *t);


/**
 * Runs all tasks and blocks until all are finished
 */
void borTasksRunBlock(bor_tasks_t *t);

/**
 * Blocks until all tasks in queue aren't finished
 */
void borTasksBarrier(bor_tasks_t *t);

// TODO: AddThreads()/RemoveThreads()

/**** INLINES ****/
_bor_inline size_t borTasksNumThreads(const bor_tasks_t *t)
{
    return t->threads_len;
}

_bor_inline size_t borTasksSize(const bor_tasks_t *t)
{
    return t->threads_len;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_TASKS_H__ */
