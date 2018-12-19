#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/segmarr.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "data.h"

TEST(segmarrTest)
{
    bor_segmarr_t *arr;
    size_t i;
    bor_vec2_t *v2;

    arr = borSegmArrNew(sizeof(bor_vec2_t), 1024);
    assertNotEquals(arr, NULL);

    for (i = 0; i < vecs2_len; ++i){
        v2 = borSegmArrGet(arr, i);
        memcpy(v2, vecs2 + i, sizeof(bor_vec2_t));
    }

    for (i = 0; i < vecs2_len; ++i){
        v2 = borSegmArrGet(arr, i);
        assertEquals(memcmp(v2, vecs2 + i, sizeof(bor_vec2_t)), 0);
    }

    // try to allocate far after the current set of segments
    v2 = borSegmArrGet(arr, 4 * vecs2_len);
    memcpy(v2, vecs2, sizeof(bor_vec2_t));
    assertEquals(memcmp(v2, vecs2, sizeof(bor_vec2_t)), 0);

    v2 = borSegmArrGet(arr, 2 * vecs2_len);
    memcpy(v2, vecs2 + 1, sizeof(bor_vec2_t));
    assertEquals(memcmp(v2, vecs2 + 1, sizeof(bor_vec2_t)), 0);

    borSegmArrDel(arr);
}
