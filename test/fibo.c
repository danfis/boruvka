#include <stdio.h>
#include "cu.h"
#include <boruvka/fibo.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>
#include <boruvka/sort.h>

/*
static void treeDump(bor_list_t *fl, size_t spaces)
{
    bor_list_t *item;
    bor_fibo_node_t *n;
    size_t i;

    BOR_LIST_FOR_EACH(fl, item){
        n = BOR_LIST_ENTRY(item, bor_fibo_node_t, list);
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

    borRandInit(&r);

    els = BOR_ALLOC_ARR(el_t, num);
    for (i = 0; i < num; i++){
        val = borRand(&r, -500., 500.);
        els[i].val = val;
        els[i].id = i;
    }

    return els;
}

static int ltEl(const bor_fibo_node_t *n1, const bor_fibo_node_t *n2, void *_)
{
    el_t *el1, *el2;

    el1 = bor_container_of(n1, el_t, node);
    el2 = bor_container_of(n2, el_t, node);

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

    sprintf(fn, "reg/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    fibo = borFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borFiboAdd(fibo, &els[i].node);
    }

    i = 0;
    while (!borFiboEmpty(fibo)){
        n = borFiboExtractMin(fibo);
        //if (num == 100)
        //    fiboDump(fibo);
        el = bor_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    BOR_SORT_BY_INT_KEY(els, num, el_t, val);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borFiboDel(fibo);
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

    borRandInit(&r);

    sprintf(fn, "reg/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    fibo = borFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borFiboAdd(fibo, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val -= borRand(&r, 1, 100);
        borFiboDecreaseKey(fibo, &els[i].node);
    }

    i = 0;
    while (!borFiboEmpty(fibo)){
        n = borFiboExtractMin(fibo);
        el = bor_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    BOR_SORT_BY_INT_KEY(els, num, el_t, val);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borFiboDel(fibo);
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

    borRandInit(&r);

    sprintf(fn, "reg/tmp.TSFibo.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSFibo.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    fibo = borFiboNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borFiboAdd(fibo, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val += borRand(&r, 1, 100);
        borFiboUpdate(fibo, &els[i].node);
    }

    i = 0;
    while (!borFiboEmpty(fibo)){
        n = borFiboExtractMin(fibo);
        el = bor_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    BOR_SORT_BY_INT_KEY(els, num, el_t, val);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borFiboDel(fibo);
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

