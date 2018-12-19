#include <stdio.h>
#include "cu.h"
#include <boruvka/pairheap.h>
#include <boruvka/pairheap_nonintrusive_int.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/sort.h>

struct _el_t {
    int val;
    bor_pairheap_node_t node;
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

static int ltEl(const bor_pairheap_node_t *n1, const bor_pairheap_node_t *n2, void *_)
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
    bor_pairheap_t *heap;
    bor_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];

    sprintf(fn, "reg/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borPairHeapNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borPairHeapAdd(heap, &els[i].node);
    }

    i = 0;
    while (!borPairHeapEmpty(heap)){
        n = borPairHeapExtractMin(heap);
        //if (num == 100)
        //    heapDump(heap);
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


    borPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

static void checkCorrect2(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_pairheap_t *heap;
    bor_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    borRandInit(&r);

    sprintf(fn, "reg/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borPairHeapNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borPairHeapAdd(heap, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val -= borRand(&r, 1, 100);
        borPairHeapDecreaseKey(heap, &els[i].node);
    }

    i = 0;
    while (!borPairHeapEmpty(heap)){
        n = borPairHeapExtractMin(heap);
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


    borPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}


static void checkCorrect3(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_pairheap_t *heap;
    bor_pairheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    borRandInit(&r);

    sprintf(fn, "reg/tmp.TSPairHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSPairHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borPairHeapNew(ltEl, NULL);
    for (i = 0; i < num; i++){
        borPairHeapAdd(heap, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val += borRand(&r, 1, 100);
        borPairHeapUpdate(heap, &els[i].node);
    }

    i = 0;
    while (!borPairHeapEmpty(heap)){
        n = borPairHeapExtractMin(heap);
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


    borPairHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

static void checkCorrectNonIntrusiveInt(int ID, size_t num)
{
    el_t *els, *el;
    int key;
    bor_pairheap_nonintr_int_t *heap;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];

    sprintf(fn, "reg/tmp.TSPairHeapNonIntrInt.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "reg/TSPairHeapNonIntrInt.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);

    heap = borPairHeapNonIntrIntNew();
    for (i = 0; i < num; i++){
        borPairHeapNonIntrIntAdd(heap, els[i].val, &els[i]);
    }

    i = 0;
    while (!borPairHeapNonIntrIntEmpty(heap)){
        el = borPairHeapNonIntrIntExtractMin(heap, &key);
        //if (num == 100)
        //    heapDump(heap);
        assertEquals(el->val, key);
        fprintf(fout1, "%d\n", el->val);
    }

    BOR_SORT_BY_INT_KEY(els, num, el_t, val);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }

    borPairHeapNonIntrIntDel(heap);
    free(els);

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

static void clearFn(bor_pairheap_node_t *node,
                    void *data)
{
    el_t *el = bor_container_of(node, el_t, node);
    BOR_FREE(el);
}

TEST(pairheapClear)
{
    size_t i, size = 10000;
    el_t *el;
    bor_pairheap_t *heap;

    heap = borPairHeapNew(ltEl, NULL);

    for (i = 0; i < size; ++i){
        el = BOR_ALLOC(el_t);
        el->val = i;
        borPairHeapAdd(heap, &el->node);
    }

    borPairHeapClear(heap, clearFn, NULL);

    borPairHeapDel(heap);
}

TEST(pairheapNonIntrusiveInt)
{
    checkCorrectNonIntrusiveInt(1, 500);
    checkCorrectNonIntrusiveInt(2, 5000);

    bor_pairheap_nonintr_int_t *heap;

    heap = borPairHeapNonIntrIntNew();
    borPairHeapNonIntrIntAdd(heap, 1, (void *)0x1);
    borPairHeapNonIntrIntAdd(heap, 2, (void *)0x2);
    borPairHeapNonIntrIntAdd(heap, 0, (void *)0x3);
    borPairHeapNonIntrIntAdd(heap, 10, (void *)0x4);
    borPairHeapNonIntrIntAdd(heap, 11, (void *)0x5);
    borPairHeapNonIntrIntDel(heap);
}
