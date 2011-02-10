#include <stdio.h>
#include "cu.h"
#include <fermat/pairheap.h>
#include <fermat/rand.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

struct _el_t {
    int val;
    fer_pairheap_node_t node;
    int id;
};
typedef struct _el_t el_t;

static el_t *randomEls(size_t num)
{
    fer_rand_t r;
    fer_real_t val;
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

static int ltEl(const fer_pairheap_node_t *n1, const fer_pairheap_node_t *n2)
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
    fer_pairheap_t *heap;
    fer_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];

    sprintf(fn, "regressions/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    heap = ferPairHeapNew(ltEl);
    for (i = 0; i < num; i++){
        ferPairHeapAdd(heap, &els[i].node);
    }

    i = 0;
    while (!ferPairHeapEmpty(heap)){
        n = ferPairHeapExtractMin(heap);
        //if (num == 100)
        //    heapDump(heap);
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


    ferPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

static void checkCorrect2(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    fer_pairheap_t *heap;
    fer_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    fer_rand_t r;

    ferRandInit(&r);

    sprintf(fn, "regressions/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    heap = ferPairHeapNew(ltEl);
    for (i = 0; i < num; i++){
        ferPairHeapAdd(heap, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val -= ferRand(&r, 1, 100);
        ferPairHeapDecreaseKey(heap, &els[i].node);
    }

    i = 0;
    while (!ferPairHeapEmpty(heap)){
        n = ferPairHeapExtractMin(heap);
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


    ferPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}


static void checkCorrect3(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    fer_pairheap_t *heap;
    fer_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    fer_rand_t r;

    ferRandInit(&r);

    sprintf(fn, "regressions/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = FER_ALLOC_ARR(int, num);

    heap = ferPairHeapNew(ltEl);
    for (i = 0; i < num; i++){
        ferPairHeapAdd(heap, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val += ferRand(&r, 1, 100);
        ferPairHeapUpdate(heap, &els[i].node);
    }

    i = 0;
    while (!ferPairHeapEmpty(heap)){
        n = ferPairHeapExtractMin(heap);
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


    ferPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

TEST(pairheap1)
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

