#include <stdio.h>
#include <limits.h>
#include <cu/cu.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>
#include <boruvka/splaytree.h>
#include <boruvka/sort.h>

struct _el_t {
    int val;
    bor_splaytree_node_t node;
    int id;
    int ins;
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
        val = borRand(&r, -50000., 50000.);
        els[i].val = val;
        els[i].id = i;
    }

    return els;
}

static int sortCmpAsc(const void *i1, const void *i2, void *_)
{
    int a = *(int *)i1;
    int b = *(int *)i2;
    return a - b;
}

static int sortCmpDesc(const void *i1, const void *i2, void *_)
{
    int a = *(int *)i1;
    int b = *(int *)i2;
    return b - a;
}

static void checkOrderAsc(bor_splaytree_t *splaytree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_splaytree_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpAsc, NULL);

    node = borSplayTreeMin(splaytree);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_SPLAYTREE_FOR_EACH(splaytree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static void checkOrderDesc(bor_splaytree_t *splaytree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_splaytree_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpDesc, NULL);

    node = borSplayTreeMin(splaytree);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_SPLAYTREE_FOR_EACH_REVERSE(splaytree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static int stCmp(const bor_splaytree_node_t *n1,
                 const bor_splaytree_node_t *n2,
                 void *_)
{
    el_t *e1 = bor_container_of(n1, el_t, node);
    el_t *e2 = bor_container_of(n2, el_t, node);
    return e1->val - e2->val;
}

TEST(splaytreeInsert)
{
    el_t *els;
    size_t i, size = 10000;
    bor_splaytree_t *splaytree;

    splaytree = borSplayTreeNew(stCmp, NULL);
    assertTrue(borSplayTreeEmpty(splaytree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        borSplayTreeInsert(splaytree, &els[i].node);
        assertFalse(borSplayTreeEmpty(splaytree));
    }
    checkOrderAsc(splaytree, els, size);
    checkOrderDesc(splaytree, els, size);

    BOR_FREE(els);
    borSplayTreeDel(splaytree);
}

TEST(splaytreeRemove)
{
    el_t *els;
    size_t i, size = 10000;
    bor_splaytree_t *splaytree;
    bor_splaytree_node_t *n, *tmpn;

    splaytree = borSplayTreeNew(stCmp, NULL);
    assertTrue(borSplayTreeEmpty(splaytree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borSplayTreeInsert(splaytree, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    assertFalse(borSplayTreeEmpty(splaytree));
    for (i = 0; i < size; ++i){
        if (!els[i].ins)
            continue;
        borSplayTreeRemove(splaytree, &els[i].node);
        n = borSplayTreeFind(splaytree, &els[i].node);
        assertEquals(n, NULL);
    }
    assertTrue(borSplayTreeEmpty(splaytree));

    for (i = 0; i < size; ++i){
        n = borSplayTreeInsert(splaytree, &els[i].node);
    }

    BOR_SPLAYTREE_FOR_EACH_SAFE(splaytree, n, tmpn){
        borSplayTreeRemove(splaytree, n);
    }
    assertTrue(borSplayTreeEmpty(splaytree));

    BOR_FREE(els);
    borSplayTreeDel(splaytree);
}

TEST(splaytreeFind)
{
    el_t *els, *el;
    size_t i, r, j, size = 10000;
    bor_rand_t rnd;
    bor_splaytree_t *splaytree;
    bor_splaytree_node_t *n, *tmpn;

    borRandInit(&rnd);

    splaytree = borSplayTreeNew(stCmp, NULL);

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borSplayTreeInsert(splaytree, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    for (i = 0; i < size; ++i){
        r = borRand(&rnd, 0., size);
        n = borSplayTreeFind(splaytree, &els[r].node);
        assertNotEquals(n, NULL);
        el = bor_container_of(n, el_t, node);
        assertEquals(el->val, els[r].val);
        for (j = 0; j < size; ++j){
            if (els[j].val == el->val){
                assertTrue(els[j].ins);
                break;
            }
        }
    }


    BOR_SPLAYTREE_FOR_EACH_REVERSE_SAFE(splaytree, n, tmpn){
        borSplayTreeRemove(splaytree, n);
    }
    assertTrue(borSplayTreeEmpty(splaytree));

    BOR_FREE(els);
    borSplayTreeDel(splaytree);
}

TEST(splaytreeExtractMin)
{
    el_t *els;
    size_t i, size = 10000;
    bor_splaytree_t *splaytree;

    splaytree = borSplayTreeNew(stCmp, NULL);
    assertTrue(borSplayTreeEmpty(splaytree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        borSplayTreeInsert(splaytree, &els[i].node);
        assertFalse(borSplayTreeEmpty(splaytree));
    }

    int last_val = INT_MIN;
    while (!borSplayTreeEmpty(splaytree)){
        bor_splaytree_node_t *stn = borSplayTreeExtractMin(splaytree);
        el_t *n = bor_container_of(stn, el_t, node);
        assertTrue(n->val >= last_val);
        last_val = n->val;
    }
    assertTrue(borSplayTreeEmpty(splaytree));

    BOR_FREE(els);
    borSplayTreeDel(splaytree);
}
