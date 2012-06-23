#include <stdio.h>
#include "cu.h"
#include <boruvka/fibo.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>

/*
static void treeDump(bor_list_t *fl, size_t spaces)
{
    bor_list_t *item;
    bor_fibo_node_t *n;
    size_t i;

    FER_LIST_FOR_EACH(fl, item){
        n = FER_LIST_ENTRY(item, bor_fibo_node_t, list);
        for (i = 0; i < spaces; i++){
            fprintf(stdout, " ");
        }
        fprintf(stdout, "%lx (%d, %d %d)\n", (long)n, n->value, n->degree, n->mark);
        treeDump(&n->children, spaces + 1);
    }
}

static void fiboDump(bor_fibo_t *fibo)
{
    fprintf(stdout, "---\n");
    fprintf(stdout, "min: %lx\n", (long)fibo->min);
    treeDump(&fibo->root, 0);
    fflush(stdout);
}
*/

struct _el_t {
    int val;
    bor_fibo_node_t node;
    int id;
};
typedef struct _el_t el_t;

static el_t *randomEls(size_t num)
{
    bor_rand_t r;
    bor_real_t val;
    el_t *els;
    size_t i;

    ferRandInit(&r);

    els = FER_ALLOC_ARR(el_t, num);
    for (i = 0; i < num; i++){
        val = ferRand(&r, -500., 500.);
        els[i].val = val;
        els[i].id = i;
    }

    return els;
}

static int cmpIncEl(const void *i1, const void *i2)
{
    el_t *e1, *e2;
    e1 = (el_t *)i1;
    e2 = (el_t *)i2;

    if (ferEq(e1->val, e2->val))
        return 0;
    if (e1->val < e2->val)
        return -1;
    return 1;
}

static int ltEl(const bor_fibo_node_t *n1, const bor_fibo_node_t *n2, void *_)
{
    el_t *el1, *el2;

    el1 = fer_container_of(n1, el_t, node);
    el2 = fer_container_of(n2, el_t, node);

    return el1->val < el2->val;
}


static void checkCorrect(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_fibo_t *fibo;
    bor_fibo_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];

    sprintf(fn, "regressions/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    fibo = ferFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        ferFiboAdd(fibo, &els[i].node);
    }

    i = 0;
    while (!ferFiboEmpty(fibo)){
        n = ferFiboExtractMin(fibo);
        //if (num == 100)
        //    fiboDump(fibo);
        el = fer_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = fer_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    ferFiboDel(fibo);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

static void checkCorrect2(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_fibo_t *fibo;
    bor_fibo_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    ferRandInit(&r);

    sprintf(fn, "regressions/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    fibo = ferFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        ferFiboAdd(fibo, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val -= ferRand(&r, 1, 100);
        ferFiboDecreaseKey(fibo, &els[i].node);
    }

    i = 0;
    while (!ferFiboEmpty(fibo)){
        n = ferFiboExtractMin(fibo);
        el = fer_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = fer_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    ferFiboDel(fibo);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}


static void checkCorrect3(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_fibo_t *fibo;
    bor_fibo_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    ferRandInit(&r);

    sprintf(fn, "regressions/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    fibo = ferFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        ferFiboAdd(fibo, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val += ferRand(&r, 1, 100);
        ferFiboUpdate(fibo, &els[i].node);
    }

    i = 0;
    while (!ferFiboEmpty(fibo)){
        n = ferFiboExtractMin(fibo);
        el = fer_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = fer_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    ferFiboDel(fibo);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

TEST(fibo1)
{
    //checkCorrect(0, 100);

    checkCorrect(1, 5000);
    checkCorrect(2, 50000);
    checkCorrect(3, 500000);

    checkCorrect2(4, 5000);
    checkCorrect2(5, 50000);
    checkCorrect2(6, 500000);

    checkCorrect3(7, 5000);
    checkCorrect3(8, 50000);
    checkCorrect3(9, 500000);
}

