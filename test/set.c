#include <stdio.h>
#include <cu/cu.h>
#include "boruvka/rand.h"
#include "boruvka/iset.h"
#include "boruvka/lset.h"
#include "boruvka/cset.h"
#include "boruvka/hashset.h"

bor_rand_t rnd;

static void testIntersection(int rng)
{
    bor_iset_t s[3], sint;
    int size, v;

    for (int si = 0; si < 3; ++si){
        size = (int)borRand(&rnd, 1, 30);
        borISetInit(&s[si]);
        for (int i = 0; i < size; ++i){
            v = (int)borRand(&rnd, -rng, rng);
            borISetAdd(&s[si], v);
        }
    }

    borISetInit(&sint);
    for (int i1 = 0; i1 < s[0].size; ++i1){
        for (int i2 = 0; i2 < s[1].size; ++i2){
            for (int i3 = 0; i3 < s[2].size; ++i3){
                if (s[0].s[i1] == s[1].s[i2] && s[0].s[i1] == s[2].s[i3])
                    borISetAdd(&sint, s[0].s[i1]);
            }
        }
    }

    for (int i = 1; i < 30; i++){
        int siz = borISetIntersectionSizeAtLeast3(s + 0, s + 1, s + 2, i);
        if (sint.size >= i){
            assertTrue(siz);
        }else{
            assertFalse(siz);
        }
    }

    borISetFree(&sint);
    for (int si = 0; si < 3; ++si)
        borISetFree(&s[si]);
}

TEST(testISet)
{
    bor_iset_t s, s1, s2, s3;

    borRandInit(&rnd);

    borISetInit(&s);
    borISetInit(&s1);
    borISetInit(&s2);
    borISetInit(&s3);

    assertEquals(borISetSize(&s), 0);
    borISetAdd(&s, 1);
    borISetAdd(&s, 1);
    assertEquals(borISetSize(&s), 1);
    borISetAdd(&s, -10);
    borISetAdd(&s, 12);
    borISetAdd(&s, 2);
    borISetAdd(&s, 1);
    borISetAdd(&s, 3);
    assertEquals(borISetSize(&s), 5);
    assertEquals(borISetGet(&s, 0), -10);
    assertEquals(borISetGet(&s, 1), 1);
    assertEquals(borISetGet(&s, 2), 2);
    assertEquals(borISetGet(&s, 3), 3);
    assertEquals(borISetGet(&s, 4), 12);
    assertTrue(borISetIsSubset(&s1, &s));
    assertFalse(borISetIsSubset(&s, &s1));

    borISetAdd(&s1, 1);
    borISetAdd(&s1, 12);
    borISetAdd(&s1, -10);
    borISetAdd(&s1, 5);
    borISetAdd(&s1, 1);
    borISetAdd(&s1, 4);
    assertEquals(borISetSize(&s1), 5);
    assertFalse(borISetIsSubset(&s1, &s));

    borISetUnion(&s2, &s1);
    assertEquals(borISetSize(&s2), 5);
    assertTrue(borISetEq(&s1, &s2));

    borISetMinus2(&s3, &s2, &s);
    borISetMinus(&s2, &s);
    assertEquals(borISetSize(&s2), 2);
    assertEquals(borISetGet(&s2, 0), 4);
    assertEquals(borISetGet(&s2, 1), 5);
    assertEquals(borISetSize(&s3), 2);
    assertEquals(borISetGet(&s3, 0), 4);
    assertEquals(borISetGet(&s3, 1), 5);

    borISetRm(&s2, 1);
    assertEquals(borISetSize(&s2), 2);
    assertEquals(borISetGet(&s2, 0), 4);
    assertEquals(borISetGet(&s2, 1), 5);

    borISetRm(&s2, 4);
    assertEquals(borISetSize(&s2), 1);
    assertEquals(borISetGet(&s2, 1), 5);
    assertFalse(borISetEq(&s2, &s));
    assertTrue(borISetHas(&s2, 5));
    assertFalse(borISetHas(&s2, 4));

    borISetIntersect(&s, &s1);
    assertEquals(borISetSize(&s), 3);
    assertEquals(borISetGet(&s, 0), -10);
    assertEquals(borISetGet(&s, 1), 1);
    assertEquals(borISetGet(&s, 2), 12);

    borISetIntersect(&s1, &s2);
    assertEquals(borISetSize(&s1), 1);
    assertEquals(borISetGet(&s1, 0), 5);

    borISetEmpty(&s);
    borISetEmpty(&s1);
    borISetEmpty(&s2);
    assertEquals(borISetSize(&s), 0);

    borISetAdd(&s, 2);
    borISetAdd(&s, 0);
    borISetAdd(&s1, 2);
    borISetAdd(&s1, 0);
    borISetAdd(&s1, 1);
    assertTrue(borISetIsSubset(&s, &s1));
    borISetAdd(&s, 4);
    borISetAdd(&s, 5);
    assertEquals(borISetIntersectionSize(&s, &s1), 2);
    assertTrue(borISetIntersectionSizeAtLeast(&s, &s1, 1));
    assertFalse(borISetIntersectionSizeAtLeast(&s, &s1, 3));

    borISetAdd(&s, -10);
    borISetAdd(&s, -20);
    borISetAdd(&s, -1);
    borISetAdd(&s, 12);
    borISetAdd(&s, 128);
    borISetAdd(&s, 128);
    borISetAdd(&s, 228);
    borISetAdd(&s1, 12);
    borISetAdd(&s1, -1);
    borISetAdd(&s1, 228);
    borISetAdd(&s1, -30);
    borISetAdd(&s1, 111);
    assertEquals(borISetIntersectionSize(&s, &s1), 5);
    assertTrue(borISetIntersectionSizeAtLeast(&s, &s1, 1));
    assertTrue(borISetIntersectionSizeAtLeast(&s, &s1, 3));
    assertFalse(borISetIntersectionSizeAtLeast(&s, &s1, 6));

    borISetFree(&s);
    borISetFree(&s1);
    borISetFree(&s2);
    borISetFree(&s3);

    for (int i = 0; i < 100; ++i)
        testIntersection(2);
    for (int i = 0; i < 100; ++i)
        testIntersection(4);
    for (int i = 0; i < 100; ++i)
        testIntersection(6);
    for (int i = 0; i < 100; ++i)
        testIntersection(10);
}

TEST(testHashSet)
{
    BOR_ISET(iset);
    bor_hashset_t hs;
    int id, id2;

    borHashSetInitISet(&hs);
    BOR_ISET_SET(&iset, 1, 2, 3);
    id = borHashSetAdd(&hs, &iset);
    id2 = borHashSetAdd(&hs, &iset);
    assertEquals(id, id2);
    BOR_ISET_SET(&iset, 1, 8, -3);
    id = borHashSetAdd(&hs, &iset);
    BOR_ISET_SET(&iset, 1, 3, 3, 8, 1, 9, 4);
    id = borHashSetAdd(&hs, &iset);
    assertEquals(hs.size, 3);
    BOR_ISET_SET(&iset, 1, 8, -3);
    id = borHashSetAdd(&hs, &iset);
    assertEquals(id, 1);
    assertEquals(id, borHashSetFind(&hs, &iset));
    const bor_iset_t *f = borHashSetGet(&hs, 1);
    assertTrue(borISetEq(f, &iset));
    assertTrue(f != &iset);

    borHashSetFree(&hs);
    borISetFree(&iset);


    BOR_LSET(lset);

    borHashSetInitLSet(&hs);
    BOR_LSET_SET(&lset, 1, 2, 3);
    id = borHashSetAdd(&hs, &lset);
    id2 = borHashSetAdd(&hs, &lset);
    assertEquals(id, id2);
    BOR_LSET_SET(&lset, 1, 8, -3);
    id = borHashSetAdd(&hs, &lset);
    BOR_LSET_SET(&lset, 1, 3, 3, 8, 1, 9, 4);
    id = borHashSetAdd(&hs, &lset);
    assertEquals(hs.size, 3);
    BOR_LSET_SET(&lset, 1, 8, -3);
    id = borHashSetAdd(&hs, &lset);
    assertEquals(id, 1);
    assertEquals(id, borHashSetFind(&hs, &lset));
    const bor_lset_t *g = borHashSetGet(&hs, 1);
    assertTrue(borLSetEq(g, &lset));
    assertTrue(g != &lset);

    borHashSetFree(&hs);
    borLSetFree(&lset);


    BOR_CSET(cset);

    borHashSetInitCSet(&hs);
    BOR_CSET_SET(&cset, 1, 2, 3);
    id = borHashSetAdd(&hs, &cset);
    id2 = borHashSetAdd(&hs, &cset);
    assertEquals(id, id2);
    BOR_CSET_SET(&cset, 1, 8, -3);
    id = borHashSetAdd(&hs, &cset);
    BOR_CSET_SET(&cset, 1, 3, 3, 8, 1, 9, 4);
    id = borHashSetAdd(&hs, &cset);
    assertEquals(hs.size, 3);
    BOR_CSET_SET(&cset, 1, 8, -3);
    id = borHashSetAdd(&hs, &cset);
    assertEquals(id, 1);
    assertEquals(id, borHashSetFind(&hs, &cset));
    const bor_cset_t *h = borHashSetGet(&hs, 1);
    assertTrue(borCSetEq(h, &cset));
    assertTrue(h != &cset);

    borHashSetFree(&hs);
    borCSetFree(&cset);
}
