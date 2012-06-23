#include <cu/cu.h>
#include <boruvka/tasks.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>

static void taskV(int id, void *data, const bor_tasks_thinfo_t *info)
{
    bor_vec3_t v, w;

    printf("[%02d]: ID: %d (%lx)\n", info->id, id, (long)data);

    ferVec3Set(&v, id, id + 1, id + 2);
    ferVec3Set(&w, id, id + 1, id + 2);

    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    ferVec3Add(&v, &w);
    printf("[%02d]: v: ", info->id);
    ferVec3Print(&v, stdout);
    printf("\n");
    fflush(stdout);
}

TEST(tasks1)
{
    bor_tasks_t *t;
    size_t c = 1;

    t = ferTasksNew(3);
    for (c = 1; c < 10; c++){
        ferTasksAdd(t, taskV, c, (void *)(long)c);
    }

    ferTasksRun(t);
    for (; c < 100; c++){
        ferTasksAdd(t, taskV, c, (void *)(long)c);
    }

    printf("===== DEL =====\n");
    fflush(stdout);
    ferTasksDel(t);
}
