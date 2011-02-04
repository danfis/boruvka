#include <cu/cu.h>
#include <fermat/nearest/linear.h>
#include <fermat/rand.h>
#include <fermat/dbg.h>

#define LINEAR_LEN 20

struct linear_t {
    fer_real_t num;
    fer_list_t list;
};

static fer_rand_t r;
static void randList(fer_list_t *head)
{
    fer_list_t *item;
    struct linear_t *it;

    ferListForEach(head, item){
        it = ferListEntry(item, struct linear_t, list);
        it->num = ferRand(&r, -10, 10);
    }
}

static fer_real_t linearDist(void *item1, fer_list_t *item2)
{
    fer_real_t r1, r2;
    struct linear_t *it;

    r1 = *(fer_real_t *)item1;
    it = ferListEntry(item2, struct linear_t, list);
    r2 = it->num;

    return FER_CUBE(r1 - r2);
}

/*
static void prList(fer_list_t *head)
{
    fer_list_t *item;
    struct linear_t *it;

    fprintf(stdout, "#");
    ferListForEach(head, item){
        it = ferListEntry(item, struct linear_t, list);
        fprintf(stdout, " %g", it->num);
    }
    fprintf(stdout, "\n");
}

static void prNearest(fer_list_t **nearest, size_t len)
{
    struct linear_t *it;
    size_t i;

    fprintf(stdout, "#");
    for (i = 0; i < len; i++){
        it = ferListEntry(nearest[i], struct linear_t, list);
        fprintf(stdout, " %g", it->num);
    }
    fprintf(stdout, "\n");
}
*/

TEST(nearestLinear)
{
    struct linear_t list, items[LINEAR_LEN];
    size_t i, len;
    fer_list_t *nearest[10];
    fer_real_t p;

    ferRandInit(&r);

    ferListInit(&list.list);
    for  (i = 0; i < LINEAR_LEN; i++){
        ferListInit(&items[i].list);
        ferListAppend(&list.list, &items[i].list);
    }

    p = 10.;
    randList(&list.list);
    len = ferNearestLinear(&list.list, &p, linearDist, nearest, 2);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 2);

    p = -1.;
    randList(&list.list);
    len = ferNearestLinear(&list.list, &p, linearDist, nearest, 4);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 4);

    p = 3.5;
    randList(&list.list);
    len = ferNearestLinear(&list.list, &p, linearDist, nearest, 3);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 3);
}
