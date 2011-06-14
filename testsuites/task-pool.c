#include <cu/cu.h>
#include <fermat/task-pool.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>

static void taskV(int id, void *data, const fer_task_pool_thinfo_t *info)
{
    printf("[%02d]: ID: %d (%lx)\n", info->id, id, (long)data);
    fflush(stdout);
}

TEST(taskpool1)
{
    fer_task_pool_t *t;
    size_t c = 1;

    printf(" === taskpool1 ===\n");

    t = ferTaskPoolNew(3);
    for (c = 1; c < 5; c++){
        ferTaskPoolAdd(t, 0, taskV, c, (void *)(long)c);
    }

    ferTaskPoolRun(t);
    ferTaskPoolBarrier(t, 0);

    for (c = 1; c < 5; c++){
        ferTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
    }
    ferTaskPoolBarrier(t, 0); // to be sure it doesn't block
    ferTaskPoolBarrier(t, 1);

    for (c = 1; c < 5; c++){
        ferTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
    }
    ferTaskPoolBarrier(t, 2);

    ferTaskPoolDel(t);

    printf(" === taskpool1 end ===\n");
}

TEST(taskpool2)
{
    fer_task_pool_t *t;
    size_t c = 1;

    printf(" === taskpool2 ===\n");

    t = ferTaskPoolNew(3);
    for (c = 1; c < 5; c++){
        ferTaskPoolAdd(t, 0, taskV, c, (void *)(long)c);
        ferTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
        ferTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
    }

    ferTaskPoolRun(t);
    ferTaskPoolBarrier(t, 0);
    ferTaskPoolBarrier(t, 1);
    ferTaskPoolBarrier(t, 2);

    printf(" ---\n");
    for (c = 1; c < 5; c++){
        ferTaskPoolAdd(t, 2, taskV, c, (void *)(long)c);
        ferTaskPoolAdd(t, 1, taskV, c, (void *)(long)c);
    }
    ferTaskPoolBarrier(t, 0);
    ferTaskPoolBarrier(t, 1);
    ferTaskPoolBarrier(t, 2);

    ferTaskPoolDel(t);

    printf(" === taskpool2 end ===\n");
}
