#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/hmap.h>
#include <boruvka/hfunc.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "data.h"

struct _el_t {
    uint32_t val;
    uint32_t id;
    bor_list_t hmap;
};
typedef struct _el_t el_t;

static el_t *els;
static size_t els_len;

static void elsInit(bor_hmap_t *m)
{
    size_t i;
    for (i = 0; i < vecs2_len; ++i){
        els[i].val = borVec2X(&vecs2[i]) * 1000000;
        borListInit(&els[i].hmap);
        els[i].id = borHMapID(m, &els[i].hmap);
    }

    els_len = vecs2_len;
}

static uint32_t hash(bor_list_t *key, void *ud)
{
    el_t *el = BOR_LIST_ENTRY(key, el_t, hmap);
    return borHashJenkins(&el->val, 1, 0);
}

static int eq(const bor_list_t *key1, const bor_list_t *key2, void *ud)
{
    el_t *el1 = BOR_LIST_ENTRY(key1, el_t, hmap);
    el_t *el2 = BOR_LIST_ENTRY(key2, el_t, hmap);
    return el1->val == el2->val;
}

TEST(hmapSetUp)
{
    els = BOR_ALLOC_ARR(el_t, vecs2_len);
}

TEST(hmapTearDown)
{
    BOR_FREE(els);
}

void hmapTestSize(int size, int pr_identical)
{
    bor_hmap_t *h;
    bor_list_t *lel;
    el_t *el;
    size_t i;

    if (size != 0){
        printf("---- hmapFixed %d ----\n", size);
    }else{
        printf("---- hmapResizable ----\n");
    }

    if (size == 0){
        h = borHMapNewResizable(hash, eq, NULL);
    }else{
        h = borHMapNew(size, hash, eq, NULL);
    }
    elsInit(h);

    for (i = 0; i < els_len; ++i){
        borHMapPut(h, &els[i].hmap);
        assertEquals(i + 1, h->num_elements);
        if (size == 0){
            assertTrue(h->num_elements <= h->size);
        }
    }

    for (i = 0; i < els_len; ++i){
        lel = borHMapGet(h, &els[i].hmap);
        assertNotEquals(lel, NULL);
        if (lel == NULL){
            fprintf(stderr, "i: %d, val: %lu, %lu, %lx\n",
                            (int)i, (unsigned long)els[i].val,
                            (unsigned long)hash(&els[i].hmap, NULL),
                            (unsigned long)&els[i].hmap);
            continue;
        }

        el = BOR_LIST_ENTRY(lel, el_t, hmap);
        assertEquals(el->val, els[i].val);
        assertEquals(el->id, els[i].id);
        if (el != els + i && pr_identical){
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

    borHMapDel(h);

    if (size != 0){
        printf("---- hmapFixed %d END ----\n", size);
    }else{
        printf("---- hmapResizable END ----\n");
    }
}

TEST(hmapFixed)
{
    hmapTestSize(111, 1);
    hmapTestSize(1023, 0);
    hmapTestSize(76389, 0);
}


TEST(hmapResizable)
{
    hmapTestSize(0, 0);
}
