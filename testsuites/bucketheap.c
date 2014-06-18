#include <stdio.h>
#include "cu.h"
#include <boruvka/bucketheap.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

struct _el_t {
    int val;
    bor_bucketheap_node_t node;
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
        val = borRand(&r, 0., 10000.);
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

    if (borEq(e1->val, e2->val))
        return 0;
    if (e1->val < e2->val)
        return -1;
    return 1;
}


static void checkCorrect(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_bucketheap_t *heap;
    bor_bucketheap_node_t *n;
    bor_bucketheap_key_t key;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];

    sprintf(fn, "regressions/tmp.TSBucketHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSBucketHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borBucketHeapNew();
    for (i = 0; i < num; i++){
        borBucketHeapAdd(heap, els[i].val, &els[i].node);
    }

    i = 0;
    while (!borBucketHeapEmpty(heap)){
        n = borBucketHeapExtractMin(heap, &key);
        //if (num == 100)
        //    heapDump(heap);
        el = bor_container_of(n, el_t, node);
        assertEquals(el->val, key);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borBucketHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

static void checkCorrect2(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_bucketheap_t *heap;
    bor_bucketheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    borRandInit(&r);

    sprintf(fn, "regressions/tmp.TSBucketHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSBucketHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borBucketHeapNew();
    for (i = 0; i < num; i++){
        borBucketHeapAdd(heap, els[i].val, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val -= borRand(&r, 1, 100);
        els[i].val = BOR_MAX(els[i].val, 0);
        borBucketHeapDecreaseKey(heap, &els[i].node, els[i].val);
    }

    i = 0;
    while (!borBucketHeapEmpty(heap)){
        n = borBucketHeapExtractMin(heap, NULL);
        el = bor_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borBucketHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}


static void checkCorrect3(int ID, size_t num)
{
    el_t *els, *el;
    int *ids;
    bor_bucketheap_t *heap;
    bor_bucketheap_node_t *n;
    size_t i;
    FILE *fout1, *fout2;
    char fn[300];
    bor_rand_t r;

    borRandInit(&r);

    sprintf(fn, "regressions/tmp.TSBucketHeap.rand-%d.out", ID);
    fout1 = fopen(fn, "w");
    sprintf(fn, "regressions/TSBucketHeap.rand-%d.out", ID);
    fout2 = fopen(fn, "w");

    els = randomEls(num);
    ids = BOR_ALLOC_ARR(int, num);

    heap = borBucketHeapNew();
    for (i = 0; i < num; i++){
        borBucketHeapAdd(heap, els[i].val, &els[i].node);
    }

    for (i = 0; i < num; i += 10){
        els[i].val += borRand(&r, 1, 100);
        borBucketHeapUpdate(heap, &els[i].node, els[i].val);
    }

    i = 0;
    while (!borBucketHeapEmpty(heap)){
        n = borBucketHeapExtractMin(heap, NULL);
        el = bor_container_of(n, el_t, node);
        fprintf(fout1, "%d\n", el->val);

        el = bor_container_of(n, el_t, node);
        ids[i] = el->id;
        i++;
    }

    qsort(els, num, sizeof(el_t), cmpIncEl);
    for (i = 0; i < num; i++){
        fprintf(fout2, "%d\n", els[i].val);
    }


    borBucketHeapDel(heap);
    free(els);
    free(ids);

    fclose(fout1);
    fclose(fout2);
}

TEST(bucketheap1)
{
    //checkCorrect(0, 100);

    checkCorrect(1, 5000);
    checkCorrect(2, 50000);
    checkCorrect(3, 500000);

    checkCorrect3(7, 5000);
    checkCorrect3(8, 50000);
    checkCorrect3(9, 500000);
}
