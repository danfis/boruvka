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

#include <limits.h>
#include <boruvka/task-pool.h>
#include <boruvka/alloc.h>

/** Maximal queue size */
#define QUEUE_SIZE SEM_VALUE_MAX

/** Single thread */
static bor_task_pool_thread_t *threadNew(bor_task_pool_t *t, int id);
static void threadDel(bor_task_pool_thread_t *th);
//static void threadCancel(bor_task_pool_thread_t *th);
static void threadJoin(bor_task_pool_thread_t *th);
static void threadRun(bor_task_pool_thread_t *th);
static void *threadMain(void *_th);


/** Single task */
struct _bor_task_pool_task_t {
    bor_task_pool_fn fn;    /*!< Callback */
    void *data;             /*!< Data for callback */
    int id;                 /*!< ID of task */
    int finish;             /*!< If set to 1 - thread will be closed
                                 immediatelly */
    bor_list_t list;        /*!< List for connection info tasks.tasks */
};
typedef struct _bor_task_pool_task_t bor_task_pool_task_t;


static bor_task_pool_task_t *taskNew(bor_task_pool_t *t, bor_task_pool_fn fn, void *data, int id);
static void taskDel(bor_task_pool_task_t *task);


static void _borTaskPoolAdd(bor_task_pool_t *t, int tid,
                         bor_task_pool_fn fn, void *data, int id, int finish);

bor_task_pool_t *borTaskPoolNew(size_t num_threads)
{
    bor_task_pool_t *t;
    size_t i;

    t = BOR_ALLOC(bor_task_pool_t);

    t->threads_len = num_threads;
    t->threads = BOR_ALLOC_ARR(bor_task_pool_thread_t *, t->threads_len);
    for (i = 0; i < t->threads_len; i++){
        t->threads[i] = threadNew(t, i);
    }

    t->started = 0;

    return t;
}

void borTaskPoolDel(bor_task_pool_t *t)
{
    size_t i;

    if (t->started){
        // add special tasks
        for (i = 0; i < t->threads_len; i++){
            _borTaskPoolAdd(t, i, NULL, NULL, -1, 1);
        }

        // join threads
        for (i = 0; i < t->threads_len; i++){
            threadJoin(t->threads[i]);
        }
    }

    // delete threads
    for (i = 0; i < t->threads_len; i++){
        threadDel(t->threads[i]);
    }
    BOR_FREE(t->threads);

    BOR_FREE(t);
}

void borTaskPoolAdd(bor_task_pool_t *t, int tid,
                    bor_task_pool_fn fn, int id, void *data)
{
    _borTaskPoolAdd(t, tid, fn, data, id, 0);
}

void borTaskPoolRun(bor_task_pool_t *t)
{
    size_t i;

    for (i = 0; i < t->threads_len; i++){
        threadRun(t->threads[i]);
    }

    t->started = 1;
}

int borTaskPoolPending(bor_task_pool_t *t, int id)
{
    int p;

    pthread_mutex_lock(&t->threads[id]->lock);
    p = t->threads[id]->pending;
    pthread_mutex_unlock(&t->threads[id]->lock);

    return p;
}

void borTaskPoolBarrier(bor_task_pool_t *t, int id)
{
    pthread_mutex_lock(&t->threads[id]->lock);
    if (t->threads[id]->pending != 0)
        pthread_cond_wait(&t->threads[id]->pending_cond,
                          &t->threads[id]->lock);
    pthread_mutex_unlock(&t->threads[id]->lock);
}




static bor_task_pool_thread_t *threadNew(bor_task_pool_t *t, int id)
{
    bor_task_pool_thread_t *th;

    th = BOR_ALLOC(bor_task_pool_thread_t);
    th->info.id    = id;
    th->task_pool  = t;

    borListInit(&th->tasks);

    pthread_mutex_init(&th->lock, NULL);
    sem_init(&th->full, 0, 0);
    sem_init(&th->empty, 0, QUEUE_SIZE);

    pthread_cond_init(&th->pending_cond, NULL);
    th->pending = 0;

    return th;
}

static void threadDel(bor_task_pool_thread_t *th)
{
    bor_list_t *item;
    bor_task_pool_task_t *task;

    // delete tasks
    while (!borListEmpty(&th->tasks)){
        item = borListNext(&th->tasks);
        borListDel(item);
        task = BOR_LIST_ENTRY(item, bor_task_pool_task_t, list);
        taskDel(task);
    }

    pthread_mutex_destroy(&th->lock);
    sem_destroy(&th->full);
    sem_destroy(&th->empty);

    pthread_cond_destroy(&th->pending_cond);

    BOR_FREE(th);
}

static void threadJoin(bor_task_pool_thread_t *th)
{
    pthread_join(th->th, NULL);
}

static void threadRun(bor_task_pool_thread_t *th)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&th->th, &attr, threadMain, (void *)th);
}

static void *threadMain(void *_th)
{
    bor_task_pool_thread_t *th = (bor_task_pool_thread_t *)_th;
    bor_task_pool_task_t *task;
    bor_list_t *item;
    int finish = 0;

    while (!finish) {
        // wait for available task
        sem_wait(&th->full);

        // pick up task
        pthread_mutex_lock(&th->lock);
        item = borListNext(&th->tasks);
        borListDel(item);
        task = BOR_LIST_ENTRY(item, bor_task_pool_task_t, list);
        pthread_mutex_unlock(&th->lock);

        // let know that there is free room in queue
        sem_post(&th->empty);

        if (task->finish){
            // finish this thread
            finish = 1;
        }else{
            // run task
            task->fn(task->id, task->data, &th->info);
        }

        // delete task
        taskDel(task);

        // another task finished...
        pthread_mutex_lock(&th->lock);
        --th->pending;
        if (th->pending == 0)
            pthread_cond_broadcast(&th->pending_cond);
        pthread_mutex_unlock(&th->lock);
    }

    return NULL;
}




static bor_task_pool_task_t *taskNew(bor_task_pool_t *t, bor_task_pool_fn fn, void *data, int id)
{
    bor_task_pool_task_t *task;

    task = BOR_ALLOC(bor_task_pool_task_t);
    task->fn     = fn;
    task->data   = data;
    task->id     = id;
    task->finish = 0;
    borListInit(&task->list);
    return task;
}

static void taskDel(bor_task_pool_task_t *task)
{
    BOR_FREE(task);
}



static void _borTaskPoolAdd(bor_task_pool_t *t, int tid,
                            bor_task_pool_fn fn, void *data, int id, int finish)
{
    bor_task_pool_task_t *task;

    task = taskNew(t, fn, data, id);
    task->finish = finish;

    // reserve item in queue
    sem_wait(&t->threads[tid]->empty);

    // add to queue
    pthread_mutex_lock(&t->threads[tid]->lock);
    borListAppend(&t->threads[tid]->tasks, &task->list);
    ++t->threads[tid]->pending;
    pthread_mutex_unlock(&t->threads[tid]->lock);

    // unblock waiting thread
    sem_post(&t->threads[tid]->full);
}

