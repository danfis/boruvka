#include <cu/cu.h>
#include <boruvka/tasks.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

static void taskV(int id, void *data, const bor_tasks_thinfo_t *info)
{
    bor_vec3_t v, w;

    printf("[%02d]: ID: %d (%lx)\n", info->id, id, (long)data);

    borVec3Set(&v, id, id + 1, id + 2);
    borVec3Set(&w, id, id + 1, id + 2);

    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    borVec3Add(&v, &w);
    printf("[%02d]: v: ", info->id);
    borVec3Print(&v, stdout);
    printf("\n");
    fflush(stdout);
}

TEST(tasks1)
{
    bor_tasks_t *t;
    size_t c = 1;

    t = borTasksNew(3);
    for (c = 1; c < 10; c++){
        borTasksAdd(t, taskV, c, (void *)(long)c);
    }

    borTasksRun(t);
    for (; c < 100; c++){
        borTasksAdd(t, taskV, c, (void *)(long)c);
    }

    printf("===== DEL =====\n");
    fflush(stdout);
    borTasksDel(t);
}
