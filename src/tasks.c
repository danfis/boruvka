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
struct _fer_tasks_thread_t {
    pthread_t th;            /*!< Posix thread */
    fer_tasks_thinfo_t info; /*!< Thread info, see tasks.h */
    fer_tasks_t *tasks;      /*!< Reference to task queue */
    fer_list_t list;         /*!< Connection into task.threads list */
};
typedef struct _fer_tasks_thread_t fer_tasks_thread_t;

static fer_tasks_thread_t *threadNew(fer_tasks_t *t);
static void threadDel(fer_tasks_thread_t *th);
//static void threadCancel(fer_tasks_thread_t *th);
static void threadJoin(fer_tasks_thread_t *th);
static void threadRun(fer_tasks_thread_t *th);
static void *threadMain(void *_th);


/** Single task */
struct _fer_tasks_task_t {
    fer_tasks_fn fn; /*!< Callback */
    void *data;      /*!< Data for callback */
    int id;          /*!< ID of task */
    int finish;      /*!< If set to 1 - thread will be closed immediatelly */
    fer_list_t list; /*!< List for connection info tasks.tasks */
    fer_tasks_t *tasks; /*!< Reference to main struct */
};
typedef struct _fer_tasks_task_t fer_tasks_task_t;


static fer_tasks_task_t *taskNew(fer_tasks_t *t, fer_tasks_fn fn, void *data, int id);
static void taskDel(fer_tasks_task_t *task);


static void _ferTasksAdd(fer_tasks_t *t, fer_tasks_fn fn, void *data, int id,
                         int finish);

fer_tasks_t *ferTasksNew(size_t num_threads)
{
    fer_tasks_t *t;
    fer_tasks_thread_t *th;
    size_t i;

    t = FER_ALLOC(fer_tasks_t);
    ferListInit(&t->tasks);
    ferListInit(&t->threads);
    t->threads_len = 0;
    t->next_id     = 1;

    pthread_mutex_init(&t->lock, NULL);
    sem_init(&t->full, 0, 0);
    sem_init(&t->empty, 0, QUEUE_SIZE);

    for (i = 0; i < num_threads; i++){
        th = threadNew(t);
        ferListAppend(&t->threads, &th->list);
        t->threads_len++;
    }

    pthread_cond_init(&t->pending_cond, NULL);
    t->pending = 0;

    return t;
}

void ferTasksDel(fer_tasks_t *t)
{
    fer_list_t *item;
    fer_tasks_thread_t *th;
    fer_tasks_task_t *task;
    size_t i;

    // add special tasks
    for (i = 0; i < t->threads_len; i++){
        _ferTasksAdd(t, NULL, NULL, -1, 1);
    }

    // join threads
    FER_LIST_FOR_EACH(&t->threads, item){
        th = FER_LIST_ENTRY(item, fer_tasks_thread_t, list);
        threadJoin(th);
    }

    // delete threads
    while (!ferListEmpty(&t->threads)){
        item = ferListNext(&t->threads);
        ferListDel(item);
        th = FER_LIST_ENTRY(item, fer_tasks_thread_t, list);
        threadDel(th);
    }

    // delete tasks
    while (!ferListEmpty(&t->tasks)){
        item = ferListNext(&t->tasks);
        ferListDel(item);
        task = FER_LIST_ENTRY(item, fer_tasks_task_t, list);
        taskDel(task);
    }

    pthread_mutex_destroy(&t->lock);
    sem_destroy(&t->full);
    sem_destroy(&t->empty);

    pthread_cond_destroy(&t->pending_cond);

    FER_FREE(t);
}

void ferTasksCancelDel(fer_tasks_t *t)
{
    fer_list_t *item;
    fer_tasks_task_t *task;

    // empty task queue
    pthread_mutex_lock(&t->lock);
    while (!ferListEmpty(&t->tasks)){
        item = ferListNext(&t->tasks);
        ferListDel(item);
        task = FER_LIST_ENTRY(item, fer_tasks_task_t, list);
        taskDel(task);
    }
    pthread_mutex_unlock(&t->lock);

    // reset semaphores - do we need this ?
    // TODO: may be problem in .pending* staff!

    ferTasksDel(t);
}

void ferTasksAdd(fer_tasks_t *t, fer_tasks_fn fn, int id, void *data)
{
    _ferTasksAdd(t, fn, data, id, 0);
}

void ferTasksRun(fer_tasks_t *t)
{
    fer_list_t *item;
    fer_tasks_thread_t *th;

    FER_LIST_FOR_EACH(&t->threads, item){
        th = FER_LIST_ENTRY(item, fer_tasks_thread_t, list);
        threadRun(th);
    }
}

int ferTasksPending(fer_tasks_t *t)
{
    int p;

    pthread_mutex_lock(&t->lock);
    p = t->pending;
    pthread_mutex_unlock(&t->lock);

    return p;
}

void ferTasksRunBlock(fer_tasks_t *t)
{
    pthread_mutex_lock(&t->lock);
    ferTasksRun(t);
    pthread_cond_wait(&t->pending_cond, &t->lock);
    pthread_mutex_unlock(&t->lock);
}

void ferTasksBarrier(fer_tasks_t *t)
{
    pthread_mutex_lock(&t->lock);
    if (t->pending != 0)
        pthread_cond_wait(&t->pending_cond, &t->lock);
    pthread_mutex_unlock(&t->lock);
}




static fer_tasks_thread_t *threadNew(fer_tasks_t *t)
{
    fer_tasks_thread_t *th;

    th = FER_ALLOC(fer_tasks_thread_t);
    th->info.id    = t->next_id++;
    th->tasks      = t;
    ferListInit(&th->list);

    return th;
}

static void threadDel(fer_tasks_thread_t *th)
{
    FER_FREE(th);
}

static void threadJoin(fer_tasks_thread_t *th)
{
    pthread_join(th->th, NULL);
}

#if 0
static void threadCancel(fer_tasks_thread_t *th)
{
    pthread_cancel(th->th);
}
#endif


static void threadRun(fer_tasks_thread_t *th)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&th->th, &attr, threadMain, (void *)th);
}

static void *threadMain(void *_th)
{
    fer_tasks_thread_t *th = (fer_tasks_thread_t *)_th;
    fer_tasks_task_t *task;
    fer_list_t *item;
    int finish = 0;

    while (!finish) {
        // wait for available task
        sem_wait(&th->tasks->full);

        // pick up task
        pthread_mutex_lock(&th->tasks->lock);
        item = ferListNext(&th->tasks->tasks);
        ferListDel(item);
        task = FER_LIST_ENTRY(item, fer_tasks_task_t, list);
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




static fer_tasks_task_t *taskNew(fer_tasks_t *t, fer_tasks_fn fn, void *data, int id)
{
    fer_tasks_task_t *task;

    task = FER_ALLOC(fer_tasks_task_t);
    task->fn     = fn;
    task->data   = data;
    task->id     = id;
    task->finish = 0;
    task->tasks  = t;
    ferListInit(&task->list);
    return task;
}

static void taskDel(fer_tasks_task_t *task)
{
    FER_FREE(task);
}



static void _ferTasksAdd(fer_tasks_t *t, fer_tasks_fn fn, void *data, int id,
                         int finish)
{
    fer_tasks_task_t *task;

    task = taskNew(t, fn, data, id);
    task->finish = finish;

    // reserve item in queue
    sem_wait(&t->empty);

    // add to queue
    pthread_mutex_lock(&t->lock);
    ferListAppend(&t->tasks, &task->list);
    ++t->pending;
    pthread_mutex_unlock(&t->lock);

    // unblock waiting thread
    sem_post(&t->full);
}
