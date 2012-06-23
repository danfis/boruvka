#include <cu/cu.h>
#include <boruvka/task-pool.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

static void taskV(int id, void *data, const bor_task_pool_thinfo_t *info)
{
    printf("[%02d]: ID: %d (%lx)\n", info->id, id, (long)data);
    fflush(stdout);
}

TEST(taskpool1)
{
    bor_task_pool_t *t;
    size_t c = 1;

    printf(" === taskpool1 ===\n");

    t = borTaskPoolNew(3);
    for (c = 1; c < 5; c++){
        borTaskPoolAdd(t, 0, taskV, c, (void *)(long)c);
    }

    borTaskPoolRun(t);
    borTaskPoolBarrier(t, 0);

    for (c = 1; c < 5; c++){
        borTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
    }
    borTaskPoolBarrier(t, 0); // to be sure it doesn't block
    borTaskPoolBarrier(t, 1);

    for (c = 1; c < 5; c++){
        borTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
    }
    borTaskPoolBarrier(t, 2);

    borTaskPoolDel(t);

    printf(" === taskpool1 end ===\n");
}

TEST(taskpool2)
{
    bor_task_pool_t *t;
    size_t c = 1;

    printf(" === taskpool2 ===\n");

    t = borTaskPoolNew(3);
    for (c = 1; c < 5; c++){
        borTaskPoolAdd(t, 0, taskV, c, (void *)(long)c);
        borTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
        borTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
    }

    borTaskPoolRun(t);
    borTaskPoolBarrier(t, 0);
    borTaskPoolBarrier(t, 1);
    borTaskPoolBarrier(t, 2);

    printf(" ---\n");
    for (c = 1; c < 5; c++){
        borTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
        borTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
    }
    borTaskPoolBarrier(t, 0);
    borTaskPoolBarrier(t, 1);
    borTaskPoolBarrier(t, 2);

    borTaskPoolDel(t);

    printf(" === taskpool2 end ===\n");
}
