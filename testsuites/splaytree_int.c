#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>
#include <boruvka/splaytree_int.h>
#include <boruvka/sort.h>

struct _el_t {
    int val;
    bor_splaytree_int_node_t node;
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

static void checkOrderAsc(bor_splaytree_int_t *splaytree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_splaytree_int_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpAsc, NULL);

    node = borSplayTreeIntMin(splaytree);
    el = bor_container_of(node, el_t, node);
    assertEquals(node->key, el->val);
    i = 0;
    BOR_SPLAYTREE_INT_FOR_EACH(splaytree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

static void checkOrderDesc(bor_splaytree_int_t *splaytree, el_t *els, size_t num)
{
    int *vals, curval;
    size_t i;
    bor_splaytree_int_node_t *node;
    el_t *el;

    vals = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; ++i){
        vals[i] = els[i].val;
    }
    borSort(vals, num, sizeof(int), sortCmpDesc, NULL);

    node = borSplayTreeIntMin(splaytree);
    el = bor_container_of(node, el_t, node);
    assertEquals(node->key, el->val);
    i = 0;
    BOR_SPLAYTREE_INT_FOR_EACH_REVERSE(splaytree, node){
        curval = vals[i];
        el = bor_container_of(node, el_t, node);
        assertEquals(el->val, curval);
        for (++i; i < num && vals[i] == curval; ++i);
    }

    BOR_FREE(vals);
}

TEST(splaytreeIntInsert)
{
    el_t *els;
    size_t i, size = 10000;
    bor_splaytree_int_t *splaytree;

    splaytree = borSplayTreeIntNew();
    assertTrue(borSplayTreeIntEmpty(splaytree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        borSplayTreeIntInsert(splaytree, els[i].val, &els[i].node);
        assertFalse(borSplayTreeIntEmpty(splaytree));
    }
    checkOrderAsc(splaytree, els, size);
    checkOrderDesc(splaytree, els, size);

    BOR_FREE(els);
    borSplayTreeIntDel(splaytree);
}

TEST(splaytreeIntRemove)
{
    el_t *els;
    size_t i, size = 10000;
    bor_splaytree_int_t *splaytree;
    bor_splaytree_int_node_t *n, *tmpn;

    splaytree = borSplayTreeIntNew();
    assertTrue(borSplayTreeIntEmpty(splaytree));

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borSplayTreeIntInsert(splaytree, els[i].val, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    assertFalse(borSplayTreeIntEmpty(splaytree));
    for (i = 0; i < size; ++i){
        if (!els[i].ins)
            continue;
        borSplayTreeIntRemove(splaytree, &els[i].node);
        n = borSplayTreeIntFind(splaytree, els[i].val);
        assertEquals(n, NULL);
    }
    assertTrue(borSplayTreeIntEmpty(splaytree));

    for (i = 0; i < size; ++i){
        n = borSplayTreeIntInsert(splaytree, els[i].val, &els[i].node);
    }

    BOR_SPLAYTREE_INT_FOR_EACH_SAFE(splaytree, n, tmpn){
        borSplayTreeIntRemove(splaytree, n);
    }
    assertTrue(borSplayTreeIntEmpty(splaytree));

    BOR_FREE(els);
    borSplayTreeIntDel(splaytree);
}

TEST(splaytreeIntFind)
{
    el_t *els, *el;
    size_t i, r, j, size = 10000;
    bor_rand_t rnd;
    bor_splaytree_int_t *splaytree;
    bor_splaytree_int_node_t *n, *tmpn;

    borRandInit(&rnd);

    splaytree = borSplayTreeIntNew();

    els = randomEls(size);
    for (i = 0; i < size; ++i){
        n = borSplayTreeIntInsert(splaytree, els[i].val, &els[i].node);
        if (n == NULL){
            els[i].ins = 1;
        }else{
            els[i].ins = 0;
        }
    }

    for (i = 0; i < size; ++i){
        r = borRand(&rnd, 0., size);
        n = borSplayTreeIntFind(splaytree, els[r].val);
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


    BOR_SPLAYTREE_INT_FOR_EACH_REVERSE_SAFE(splaytree, n, tmpn){
        borSplayTreeIntRemove(splaytree, n);
    }
    assertTrue(borSplayTreeIntEmpty(splaytree));

    BOR_FREE(els);
    borSplayTreeIntDel(splaytree);
}
