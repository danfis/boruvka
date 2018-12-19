#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/vec4.h>
#include <boruvka/dbg.h>
#include "data.h"

TEST(vec4SetUp)
{
}

TEST(vec4TearDown)
{
}

TEST(vec4Alloc)
{
    bor_vec4_t *v, w;

    v = borVec4New(0., 1., 2., 3.);
    borVec4Set(&w, 0., 1., 2., 3.);
    assertTrue(borVec4Eq(v, &w));
    borVec4Del(v);
}
