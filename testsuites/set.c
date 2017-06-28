#include <stdio.h>
#include <cu/cu.h>
#include "boruvka/iset.h"
#include "boruvka/lset.h"
#include "boruvka/cset.h"

TEST(testISet)
{
    bor_iset_t s, s1, s2;

    borISetInit(&s);
    borISetInit(&s1);
    borISetInit(&s2);

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

    borISetMinus(&s2, &s);
    assertEquals(borISetSize(&s2), 2);
    assertEquals(borISetGet(&s2, 0), 4);
    assertEquals(borISetGet(&s2, 1), 5);

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
}
