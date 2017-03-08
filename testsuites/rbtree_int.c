#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>
#include <boruvka/rbtree_int.h>
#include <boruvka/sort.h>

struct _el_t {
    int val;
    bor_rbtree_int_node_t node;
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

static void checkOrder(bor_rbtree_int_t *rbtree_int, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_rbtree_int_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpAsc, NULL);

    node = borRBTreeIntMin(rbtree_int);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_RBTREE_INT_FOR_EACH(rbtree_int, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static void checkOrder2(bor_rbtree_int_t *rbtree_int, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_rbtree_int_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpDesc, NULL);

    node = borRBTreeIntMin(rbtree_int);
    el = bor_container_of(node, el_t, node);
    i = 0;
    BOR_RBTREE_INT_FOR_EACH_REVERSE(rbtree_int, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

TEST(rbtreeIntInsert)
{
    el_t *els;
    size_t i, size = 10000;
    bor_rbtree_int_t *rbtree;

    rbtree = borRBTreeIntNew();
    assertTrue(borRBTreeIntEmpty(rbtree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        borRBTreeIntInsert(rbtree, els[i].val, &els[i].node);
        assertEquals(els[i].val, borRBTreeIntKey(&els[i].node));
        assertFalse(borRBTreeIntEmpty(rbtree));
    }
    checkOrder(rbtree, els, size);
    checkOrder2(rbtree, els, size);

    BOR_FREE(els);
    borRBTreeIntDel(rbtree);
}

TEST(rbtreeIntRemove)
{
    el_t *els;
    size_t i, size = 10000;
    bor_rbtree_int_t *rbtree;
    bor_rbtree_int_node_t *n, *tmpn;

    rbtree = borRBTreeIntNew();
    assertTrue(borRBTreeIntEmpty(rbtree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borRBTreeIntInsert(rbtree, els[i].val, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    assertFalse(borRBTreeIntEmpty(rbtree));
    for (i = 0; i < size; ++i){
        if (!els[i].ins)
            continue;
        borRBTreeIntRemove(rbtree, &els[i].node);
        n = borRBTreeIntFind(rbtree, els[i].val);
        assertEquals(n, NULL);
    }
    assertTrue(borRBTreeIntEmpty(rbtree));

    for (i = 0; i < size; ++i){
        n = borRBTreeIntInsert(rbtree, els[i].val, &els[i].node);
    }

    BOR_RBTREE_INT_FOR_EACH_SAFE(rbtree, n, tmpn){
        borRBTreeIntRemove(rbtree, n);
    }
    assertTrue(borRBTreeIntEmpty(rbtree));

    BOR_FREE(els);
    borRBTreeIntDel(rbtree);
}

TEST(rbtreeIntFind)
{
    el_t *els, *el;
    size_t i, r, j, size = 10000;
    bor_rand_t rnd;
    bor_rbtree_int_t *rbtree;
    bor_rbtree_int_node_t *n, *tmpn;

    borRandInit(&rnd);

    rbtree = borRBTreeIntNew();

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borRBTreeIntInsert(rbtree, els[i].val, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    for (i = 0; i < size; ++i){
        r = borRand(&rnd, 0., size);
        n = borRBTreeIntFind(rbtree, els[r].val);
        assertNotEquals(n, NULL);
        el = bor_container_of(n, el_t, node);
        assertEquals(el->val, els[r].val);
        assertEquals(el->val, borRBTreeIntKey(&els[r].node));
        for (j = 0; j < size; ++j){
            if (els[j].val == el->val){
                assertTrue(els[j].ins);
                break;
            }
        }
    }


    BOR_RBTREE_INT_FOR_EACH_REVERSE_SAFE(rbtree, n, tmpn){
        borRBTreeIntRemove(rbtree, n);
    }
    assertTrue(borRBTreeIntEmpty(rbtree));

    BOR_FREE(els);
    borRBTreeIntDel(rbtree);
}
