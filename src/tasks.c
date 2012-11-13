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
#include <boruvka/tasks.h>
#include <boruvka/alloc.h>

/** Maximal queue size */
#define QUEUE_SIZE SEM_VALUE_MAX

/** Single thread */
struct _bor_tasks_thread_t {
    pthread_t th;            /*!< Posix thread */
    bor_tasks_thinfo_t info; /*!< Thread info, see tasks.h */
    bor_tasks_t *tasks;      /*!< Reference to task queue */
    bor_list_t list;         /*!< Connection into task.threads list */
};
typedef struct _bor_tasks_thread_t bor_tasks_thread_t;

static bor_tasks_thread_t *threadNew(bor_tasks_t *t);
static void threadDel(bor_tasks_thread_t *th);
//static void threadCancel(bor_tasks_thread_t *th);
static void threadJoin(bor_tasks_thread_t *th);
static void threadRun(bor_tasks_thread_t *th);
static void *threadMain(void *_th);


/** Single task */
struct _bor_tasks_task_t {
    bor_tasks_fn fn; /*!< Callback */
    void *data;      /*!< Data for callback */
    int id;          /*!< ID of task */
    int finish;      /*!< If set to 1 - thread will be closed immediatelly */
    bor_list_t list; /*!< List for connection info tasks.tasks */
    bor_tasks_t *tasks; /*!< Reference to main struct */
};
typedef struct _bor_tasks_task_t bor_tasks_task_t;


static bor_tasks_task_t *taskNew(bor_tasks_t *t, bor_tasks_fn fn, void *data, int id);
static void taskDel(bor_tasks_task_t *task);


static void _borTasksAdd(bor_tasks_t *t, bor_tasks_fn fn, void *data, int id,
                         int finish);

bor_tasks_t *borTasksNew(size_t num_threads)
{
    bor_tasks_t *t;
    bor_tasks_thread_t *th;
    size_t i;

    t = BOR_ALLOC(bor_tasks_t);
    borListInit(&t->tasks);
    borListInit(&t->threads);
    t->threads_len = 0;
    t->next_id     = 1;

    if (pthread_mutex_init(&t->lock, NULL) != 0)
        return NULL;
    if (sem_init(&t->full, 0, 0) != 0)
        return NULL;
    if (sem_init(&t->empty, 0, QUEUE_SIZE) != 0)
        return NULL;

    for (i = 0; i < num_threads; i++){
        th = threadNew(t);
        borListAppend(&t->threads, &th->list);
        t->threads_len++;
    }

    pthread_cond_init(&t->pending_cond, NULL);
    t->pending = 0;

    return t;
}

void borTasksDel(bor_tasks_t *t)
{
    bor_list_t *item;
    bor_tasks_thread_t *th;
    bor_tasks_task_t *task;
    size_t i;

    // add special tasks
    for (i = 0; i < t->threads_len; i++){
        _borTasksAdd(t, NULL, NULL, -1, 1);
    }

    // join threads
    BOR_LIST_FOR_EACH(&t->threads, item){
        th = BOR_LIST_ENTRY(item, bor_tasks_thread_t, list);
        threadJoin(th);
    }

    // delete threads
    while (!borListEmpty(&t->threads)){
        item = borListNext(&t->threads);
        borListDel(item);
        th = BOR_LIST_ENTRY(item, bor_tasks_thread_t, list);
        threadDel(th);
    }

    // delete tasks
    while (!borListEmpty(&t->tasks)){
        item = borListNext(&t->tasks);
        borListDel(item);
        task = BOR_LIST_ENTRY(item, bor_tasks_task_t, list);
        taskDel(task);
    }

    pthread_mutex_destroy(&t->lock);
    sem_destroy(&t->full);
    sem_destroy(&t->empty);

    pthread_cond_destroy(&t->pending_cond);

    BOR_FREE(t);
}

void borTasksCancelDel(bor_tasks_t *t)
{
    bor_list_t *item;
    bor_tasks_task_t *task;

    // empty task queue
    pthread_mutex_lock(&t->lock);
    while (!borListEmpty(&t->tasks)){
        item = borListNext(&t->tasks);
        borListDel(item);
        task = BOR_LIST_ENTRY(item, bor_tasks_task_t, list);
        taskDel(task);
    }
    pthread_mutex_unlock(&t->lock);

    // reset semaphores - do we need this ?
    // TODO: may be problem in .pending* staff!

    borTasksDel(t);
}

void borTasksAdd(bor_tasks_t *t, bor_tasks_fn fn, int id, void *data)
{
    _borTasksAdd(t, fn, data, id, 0);
}

void borTasksRun(bor_tasks_t *t)
{
    bor_list_t *item;
    bor_tasks_thread_t *th;

    BOR_LIST_FOR_EACH(&t->threads, item){
        th = BOR_LIST_ENTRY(item, bor_tasks_thread_t, list);
        threadRun(th);
    }
}

int borTasksPending(bor_tasks_t *t)
{
    int p;

    pthread_mutex_lock(&t->lock);
    p = t->pending;
    pthread_mutex_unlock(&t->lock);

    return p;
}

void borTasksRunBlock(bor_tasks_t *t)
{
    pthread_mutex_lock(&t->lock);
    borTasksRun(t);
    pthread_cond_wait(&t->pending_cond, &t->lock);
    pthread_mutex_unlock(&t->lock);
}

void borTasksBarrier(bor_tasks_t *t)
{
    pthread_mutex_lock(&t->lock);
    if (t->pending != 0)
        pthread_cond_wait(&t->pending_cond, &t->lock);
    pthread_mutex_unlock(&t->lock);
}




static bor_tasks_thread_t *threadNew(bor_tasks_t *t)
{
    bor_tasks_thread_t *th;

    th = BOR_ALLOC(bor_tasks_thread_t);
    th->info.id    = t->next_id++;
    th->tasks      = t;
    borListInit(&th->list);

    return th;
}

static void threadDel(bor_tasks_thread_t *th)
{
    BOR_FREE(th);
}

static void threadJoin(bor_tasks_thread_t *th)
{
    pthread_join(th->th, NULL);
}

#if 0
static void threadCancel(bor_tasks_thread_t *th)
{
    pthread_cancel(th->th);
}
#endif


static void threadRun(bor_tasks_thread_t *th)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&th->th, &attr, threadMain, (void *)th);
}

static void *threadMain(void *_th)
{
    bor_tasks_thread_t *th = (bor_tasks_thread_t *)_th;
    bor_tasks_task_t *task;
    bor_list_t *item;
    int finish = 0;

    while (!finish) {
        // wait for available task
        sem_wait(&th->tasks->full);

        // pick up task
        pthread_mutex_lock(&th->tasks->lock);
        item = borListNext(&th->tasks->tasks);
        borListDel(item);
        task = BOR_LIST_ENTRY(item, bor_tasks_task_t, list);
        pthread_mutex_unlock(&th->tasks->lock);

        // let know that there is free room in queue
        sem_post(&th->tasks->empty);

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
        pthread_mutex_lock(&th->tasks->lock);
        --th->tasks->pending;
        if (th->tasks->pending == 0)
            pthread_cond_broadcast(&th->tasks->pending_cond);
        pthread_mutex_unlock(&th->tasks->lock);
    }

    return NULL;
}




static bor_tasks_task_t *taskNew(bor_tasks_t *t, bor_tasks_fn fn, void *data, int id)
{
    bor_tasks_task_t *task;

    task = BOR_ALLOC(bor_tasks_task_t);
    task->fn     = fn;
    task->data   = data;
    task->id     = id;
    task->finish = 0;
    task->tasks  = t;
    borListInit(&task->list);
    return task;
}

static void taskDel(bor_tasks_task_t *task)
{
    BOR_FREE(task);
}



static void _borTasksAdd(bor_tasks_t *t, bor_tasks_fn fn, void *data, int id,
                         int finish)
{
    bor_tasks_task_t *task;

    task = taskNew(t, fn, data, id);
    task->finish = finish;

    // reserve item in queue
    sem_wait(&t->empty);

    // add to queue
    pthread_mutex_lock(&t->lock);
    borListAppend(&t->tasks, &task->list);
    ++t->pending;
    pthread_mutex_unlock(&t->lock);

    // unblock waiting thread
    sem_post(&t->full);
}
