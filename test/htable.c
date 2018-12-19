#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/htable.h>
#include <boruvka/hfunc.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "data.h"

struct _el_t {
    uint32_t val;
    bor_htable_key_t hash;
    bor_list_t htable;
};
typedef struct _el_t el_t;

static el_t *els;
static size_t els_len;

static bor_htable_key_t hash(const bor_list_t *key, void *ud)
{
    el_t *el = BOR_LIST_ENTRY(key, el_t, htable);
    return borHashJenkins(&el->val, 1, 0);
}

static int eq(const bor_list_t *key1, const bor_list_t *key2, void *ud)
{
    el_t *el1 = BOR_LIST_ENTRY(key1, el_t, htable);
    el_t *el2 = BOR_LIST_ENTRY(key2, el_t, htable);
    return el1->val == el2->val;
}

static void elsInit(void)
{
    size_t i;
    for (i = 0; i < vecs2_len; ++i){
        els[i].val = (float)borVec2X(&vecs2[i]) * 1000000;
        borListInit(&els[i].htable);
        els[i].hash = hash(&els[i].htable, NULL);
    }

    els_len = vecs2_len;
}

TEST(htableSetUp)
{
    els = BOR_ALLOC_ARR(el_t, vecs2_len);
    elsInit();
}

TEST(htableTearDown)
{
    BOR_FREE(els);
}

TEST(htableBasic)
{
    bor_htable_t *h;
    bor_list_t *lel;
    el_t *el;
    size_t i, tmp;

    printf("---- htableBasic ----\n");

    h = borHTableNew(hash, eq, NULL);

    for (i = 0; i < els_len; ++i){
        borHTableInsert(h, &els[i].htable);
        assertEquals(i + 1, h->num_elements);
        assertTrue(h->num_elements <= h->size);
    }

    for (i = 0; i < els_len; ++i){
        lel = borHTableFind(h, &els[i].htable);
        assertNotEquals(lel, NULL);
        if (lel == NULL){
            fprintf(stderr, "i: %d, val: %lu, %lu, %lx\n",
                            (int)i, (unsigned long)els[i].val,
                            (unsigned long)hash(&els[i].htable, NULL),
                            (unsigned long)&els[i].htable);
            continue;
        }

        el = BOR_LIST_ENTRY(lel, el_t, htable);
        assertEquals(el->val, els[i].val);
        assertEquals(el->hash, els[i].hash);
        if (el != els + i){
            printf("%lu %lu\n", (unsigned long)el->val,
                                (unsigned long)els[i].val);

            size_t j;
            for (j = 0; j < els_len; ++j){
                if (el->val == els[j].val){
                    printf("  --> %d\n", (int)j);
                }
            }
        }
    }

    for (i = 0; i < els_len / 2; ++i){
        tmp = h->num_elements;
        assertEquals(borHTableErase(h, &els[i].htable), 0);
        assertEquals(tmp - 1, h->num_elements);
    }

    borHTableDel(h);

    printf("---- htableBasic END ----\n");
}

TEST(htableFindAll)
{
    bor_htable_t *h;
    bor_list_t **fels;
    size_t i, size, found;

    printf("---- htableFindAll ----\n");

    h = borHTableNew(hash, eq, NULL);

    for (i = 0; i < els_len; ++i){
        borHTableInsert(h, &els[i].htable);
    }

    fels = BOR_ALLOC_ARR(bor_list_t *, 2);
    for (i = 0; i < els_len; ++i){
        size = 2;
        found = borHTableFindAll(h, &els[i].htable, &fels, &size);
        assertEquals(size, 2);
        assertTrue(found > 0);
        if (els[i].val == 891218
                || els[i].val == 743913
                || els[i].val == 1996293){
            assertEquals(found, 3);
        }
        //printf("%lu, %lu\n", (unsigned long)els[i].val, found);
    }
    BOR_FREE(fels);

    for (i = 0; i < els_len; ++i){
        size = 0;
        fels = 0x0;
        found = borHTableFindAll(h, &els[i].htable, &fels, &size);
        assertTrue(size > 0);
        assertTrue(found > 0);
        assertEquals(found, size);
        if (els[i].val == 891218
                || els[i].val == 743913
                || els[i].val == 1996293){
            assertEquals(found, 3);
        }
        if (fels)
            BOR_FREE(fels);
        //printf("%lu, %lu\n", (unsigned long)els[i].val, found);
    }

    borHTableDel(h);

    printf("---- htableFindAll END ----\n");
}

TEST(htableInsertUnique)
{
    bor_htable_t *h;
    bor_list_t **fels;
    bor_list_t *ins;
    el_t fel;
    size_t i, size, found;

    printf("---- htableInsertUnique ----\n");

    h = borHTableNew(hash, eq, NULL);

    for (i = 0; i < els_len; ++i){
        ins = borHTableInsertUnique(h, &els[i].htable);
        if ((els[i].val == 1651122 && i >= 13994)
                || (els[i].val == 743913 && i >= 80422)
                || (els[i].val == 1996293 && i >= 4423)){
            assertNotEquals(ins, NULL);
        }
    }

    fels = BOR_ALLOC_ARR(bor_list_t *, 1);
    for (i = 0; i < els_len / 2; ++i){
        size = 1;
        found = borHTableFindAll(h, &els[i].htable, &fels, &size);
        assertEquals(size, 1);
        assertEquals(found, 1);
    }
    BOR_FREE(fels);

    fel.val = 0;
    ins = borHTableFind(h, &fel.htable);
    assertEquals(ins, NULL);

    fel.val = 121;
    ins = borHTableFind(h, &fel.htable);
    assertEquals(ins, NULL);

    fel.val = 16500;
    ins = borHTableFind(h, &fel.htable);
    assertEquals(ins, NULL);

    borHTableDel(h);

    printf("---- htableInsertUnique END ----\n");
}
