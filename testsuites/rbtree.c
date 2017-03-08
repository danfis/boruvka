#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>
#include <boruvka/rbtree.h>
#include <boruvka/sort.h>

struct _el_t {
    int val;
    bor_rbtree_node_t node;
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

static void checkOrder(bor_rbtree_t *rbtree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_rbtree_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpAsc, NULL);

    node = borRBTreeMin(rbtree);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_RBTREE_FOR_EACH(rbtree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static void checkOrder2(bor_rbtree_t *rbtree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_rbtree_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpDesc, NULL);

    node = borRBTreeMin(rbtree);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_RBTREE_FOR_EACH_REVERSE(rbtree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static int rbCmp(const bor_rbtree_node_t *n1,
                 const bor_rbtree_node_t *n2,
                 void *_)
{
    el_t *e1 = bor_container_of(n1, el_t, node);
    el_t *e2 = bor_container_of(n2, el_t, node);
    return e1->val - e2->val;
}

TEST(rbtreeInsert)
{
    el_t *els;
    size_t i, size = 10000;
    bor_rbtree_t *rbtree;

    rbtree = borRBTreeNew(rbCmp, NULL);
    assertTrue(borRBTreeEmpty(rbtree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        borRBTreeInsert(rbtree, &els[i].node);
        assertFalse(borRBTreeEmpty(rbtree));
    }
    checkOrder(rbtree, els, size);
    checkOrder2(rbtree, els, size);

    BOR_FREE(els);
    borRBTreeDel(rbtree);
}

TEST(rbtreeRemove)
{
    el_t *els, *el;
    size_t i, size = 10000;
    bor_rbtree_t *rbtree;
    bor_rbtree_node_t *n, *tmpn;

    rbtree = borRBTreeNew(rbCmp, NULL);
    assertTrue(borRBTreeEmpty(rbtree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borRBTreeInsert(rbtree, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    assertFalse(borRBTreeEmpty(rbtree));
    for (i = 0; i < size; ++i){
        if (!els[i].ins)
            continue;
        borRBTreeRemove(rbtree, &els[i].node);
        n = borRBTreeFind(rbtree, &els[i].node);
        assertEquals(n, NULL);

        n = borRBTreeFindNearestGE(rbtree, &els[i].node);
        if (n != NULL){
            el = bor_container_of(n, el_t, node);
            assertTrue(el->val > els[i].val);
        }
    }
    assertTrue(borRBTreeEmpty(rbtree));

    for (i = 0; i < size; ++i){
        n = borRBTreeInsert(rbtree, &els[i].node);
    }

    BOR_RBTREE_FOR_EACH_SAFE(rbtree, n, tmpn){
        borRBTreeRemove(rbtree, n);
    }
    assertTrue(borRBTreeEmpty(rbtree));

    BOR_FREE(els);
    borRBTreeDel(rbtree);
}

TEST(rbtreeFind)
{
    el_t *els, *el;
    size_t i, r, j, size = 10000;
    bor_rand_t rnd;
    bor_rbtree_t *rbtree;
    bor_rbtree_node_t *n, *tmpn;

    borRandInit(&rnd);

    rbtree = borRBTreeNew(rbCmp, NULL);

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borRBTreeInsert(rbtree, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    for (i = 0; i < size; ++i){
        r = borRand(&rnd, 0., size);
        n = borRBTreeFind(rbtree, &els[r].node);
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


    BOR_RBTREE_FOR_EACH_REVERSE_SAFE(rbtree, n, tmpn){
        borRBTreeRemove(rbtree, n);
    }
    assertTrue(borRBTreeEmpty(rbtree));

    BOR_FREE(els);
    borRBTreeDel(rbtree);
}
