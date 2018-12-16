#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/multimap_int.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>

#define MIN_KEY_INT -50
#define MAX_KEY_INT 50

struct _el_int_t {
    int key;
    bor_multimap_int_node_t node;
    int id;
    int checked;
    int inserted;
};
typedef struct _el_int_t el_int_t;

static el_int_t *randomElInt(size_t size)
{
    el_int_t *els;
    bor_rand_t rnd;
    size_t i;

    borRandInit(&rnd);

    els = BOR_ALLOC_ARR(el_int_t, size);
    for (i = 0; i < size; ++i){
        els[i].id = i;
        els[i].key = borRand(&rnd, MIN_KEY_INT, MAX_KEY_INT);
    }

    return els;
}

static void checkIntForEach(bor_multimap_int_t *mm,
                            int key,
                            el_int_t *els,
                            size_t size)
{
    el_int_t *el;
    size_t i;
    bor_list_t *list;
    bor_multimap_int_node_t *node;

    i = 0;
    BOR_MULTIMAP_INT_FOR_EACH(mm, key, list, node){
        el = bor_container_of(node, el_int_t, node);
        assertEquals(el->key, key);

        for(; i < size && (!els[i].inserted || els[i].key != key); ++i);
        assertNotEquals(i, size);

        if (i < size){
            assertEquals(el, &els[i]);
            els[i].checked = 1;
            i++;
        }
    }
}

static void checkIntInsert(bor_multimap_int_t *mm,
                           el_int_t *els,
                           size_t size)
{
    int key;
    size_t i;
    bor_rand_t rnd;

    borRandInit(&rnd);

    for (i = 0; i < size; ++i)
        els[i].checked = 0;

    for (key = MIN_KEY_INT; key < MAX_KEY_INT + 1; ++key){
        checkIntForEach(mm, key, els, size);
    }

    for (i = 0; i < 300; ++i){
        key = borRand(&rnd, MIN_KEY_INT, MAX_KEY_INT);
        checkIntForEach(mm, key, els, size);
    }

    for (i = 0; i < size; ++i){
        if (els[i].inserted){
            assertTrue(els[i].checked);
        }
    }
}

static int minIntKey(el_int_t *els, size_t size)
{
    size_t i;
    int key = MAX_KEY_INT;

    for (i = 0; i < size; ++i){
        if (els[i].key < key)
            key = els[i].key;
    }

    return key;
}

TEST(multimapIntInsert)
{
    el_int_t *els;
    size_t i, size = 3000;
    bor_multimap_int_t *mm;

    els = randomElInt(size);
    mm = borMultiMapIntNew();

    assertTrue(borMultiMapIntEmpty(mm));
    for (i = 0; i < size; ++i){
        borMultiMapIntInsert(mm, els[i].key, &els[i].node);
        els[i].inserted = 1;
    }
    checkIntInsert(mm, els, size);
    assertFalse(borMultiMapIntEmpty(mm));

    borMultiMapIntDel(mm);
    BOR_FREE(els);
}

TEST(multimapIntRemove)
{
    el_int_t *els, *el;
    size_t i, size = 3000;
    bor_multimap_int_t *mm;
    bor_rand_t rnd;

    borRandInit(&rnd);

    els = randomElInt(size);
    mm = borMultiMapIntNew();

    assertTrue(borMultiMapIntEmpty(mm));
    for (i = 0; i < size; ++i){
        borMultiMapIntInsert(mm, els[i].key, &els[i].node);
        els[i].inserted = 1;
    }
    assertFalse(borMultiMapIntEmpty(mm));

    for (i = 0; i < BOR_MIN(size, 300); ++i){
        el = &els[(int)borRand(&rnd, 0, size)];
        if (!el->inserted)
            continue;

        borMultiMapIntRemove(mm, &el->node);
        el->inserted = 0;
        checkIntInsert(mm, els, size);
    }
    assertFalse(borMultiMapIntEmpty(mm));

    borMultiMapIntDel(mm);
    BOR_FREE(els);
}

TEST(multimapIntMin)
{
    el_int_t *els, *el;
    size_t i, size = 3000;
    bor_multimap_int_t *mm;
    bor_multimap_int_node_t *node;
    bor_rand_t rnd;
    int key;
    el_int_t els2[30];

    borRandInit(&rnd);

    els = randomElInt(size);
    mm = borMultiMapIntNew();

    assertTrue(borMultiMapIntEmpty(mm));
    for (i = 0; i < size; ++i){
        borMultiMapIntInsert(mm, els[i].key, &els[i].node);
        els[i].inserted = 1;
    }
    assertFalse(borMultiMapIntEmpty(mm));

    assertNotEquals(borMultiMapIntMin(mm, &key), NULL);
    assertEquals(key, minIntKey(els, size));

    for (i = 0; i < 10; ++i){
        els2[i].key = -30000;
        borMultiMapIntInsert(mm, els2[i].key, &els2[i].node);
    }
    for (i = 10; i < 20; ++i){
        els2[i].key = -20000;
        borMultiMapIntInsert(mm, els2[i].key, &els2[i].node);
    }
    for (i = 20; i < 30; ++i){
        els2[i].key = -10000;
        borMultiMapIntInsert(mm, els2[i].key, &els2[i].node);
    }

    for (i = 0; i < 5; ++i){
        node = borMultiMapIntExtractMinNodeFifo(mm, &key);
        assertNotEquals(node, NULL);
        el = bor_container_of(node, el_int_t, node);
        assertEquals(key, -30000);
        assertEquals(el, &els2[i]);
    }

    for (i = 0; i < 5; ++i){
        node = borMultiMapIntExtractMinNodeLifo(mm, &key);
        assertNotEquals(node, NULL);
        el = bor_container_of(node, el_int_t, node);
        assertEquals(key, -30000);
        assertEquals(el, &els2[9 - i]);
    }

    for (i = 0; i < 5; ++i){
        node = borMultiMapIntExtractMinNodeFifo(mm, &key);
        assertNotEquals(node, NULL);
        el = bor_container_of(node, el_int_t, node);
        assertEquals(key, -20000);
        assertEquals(el, &els2[10 + i]);
    }

    for (i = 0; i < 5; ++i){
        node = borMultiMapIntExtractMinNodeLifo(mm, &key);
        assertNotEquals(node, NULL);
        el = bor_container_of(node, el_int_t, node);
        assertEquals(key, -20000);
        assertEquals(el, &els2[19 - i]);
    }

    borMultiMapIntDel(mm);
    BOR_FREE(els);
}
