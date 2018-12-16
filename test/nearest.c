#include <cu/cu.h>
#include <boruvka/nearest-linear.h>
#include <boruvka/rand.h>
#include <boruvka/dbg.h>

#define LINEAR_LEN 20

struct linear_t {
    bor_real_t num;
    bor_list_t list;
};

static bor_rand_t r;
static void randList(bor_list_t *head)
{
    bor_list_t *item;
    struct linear_t *it;

    BOR_LIST_FOR_EACH(head, item){
        it = BOR_LIST_ENTRY(item, struct linear_t, list);
        it->num = borRand(&r, -10, 10);
    }
}

static bor_real_t linearDist(void *item1, bor_list_t *item2, void *_)
{
    bor_real_t r1, r2;
    struct linear_t *it;

    r1 = *(bor_real_t *)item1;
    it = BOR_LIST_ENTRY(item2, struct linear_t, list);
    r2 = it->num;

    return BOR_SQ(r1 - r2);
}

/*
static void prList(bor_list_t *head)
{
    bor_list_t *item;
    struct linear_t *it;

    fprintf(stdout, "#");
    BOR_LIST_FOR_EACH(head, item){
        it = BOR_LIST_ENTRY(item, struct linear_t, list);
        fprintf(stdout, " %g", it->num);
    }
    fprintf(stdout, "\n");
}

static void prNearest(bor_list_t **nearest, size_t len)
{
    struct linear_t *it;
    size_t i;

    fprintf(stdout, "#");
    for (i = 0; i < len; i++){
        it = BOR_LIST_ENTRY(nearest[i], struct linear_t, list);
        fprintf(stdout, " %g", it->num);
    }
    fprintf(stdout, "\n");
}
*/

TEST(nearestLinear)
{
    struct linear_t list, items[LINEAR_LEN];
    size_t i;
    bor_list_t *nearest[10];
    bor_real_t p;

    borRandInit(&r);

    borListInit(&list.list);
    for  (i = 0; i < LINEAR_LEN; i++){
        borListInit(&items[i].list);
        borListAppend(&list.list, &items[i].list);
    }

    p = 10.;
    randList(&list.list);
    borNearestLinear(&list.list, &p, linearDist, nearest, 2, NULL);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 2);

    p = -1.;
    randList(&list.list);
    borNearestLinear(&list.list, &p, linearDist, nearest, 4, NULL);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 4);

    p = 3.5;
    randList(&list.list);
    borNearestLinear(&list.list, &p, linearDist, nearest, 3, NULL);
    //prList(&list.list);
    //fprintf(stdout, "# %g\n", p);
    //prNearest(nearest, 3);
}
