#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/mat4.h>
#include <boruvka/dbg.h>
#include "data.h"

TEST(mat4SetUp)
{
}

TEST(mat4TearDown)
{
}

TEST(mat4Alloc)
{
    /*
    bor_mat4_t *v, w;

    v = borMat4New(0., 1., 2., 3.);
    borMat4Set(&w, 0., 1., 2., 3.);
    assertTrue(borMat4Eq(v, &w));
    borMat4Del(v);
    */
}

TEST(mat4Tr)
{
#ifndef BOR_SSE_SINGLE
    bor_vec3_t v, w, axis;
    bor_mat4_t tr;

    borVec3Set(&v, -1.2, 5, 2);

    borMat4SetIdentity(&tr);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borVec3Eq(&w, &v));

    borMat4SetScale(&tr, 2.);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), 2. * -1.2));
    assertTrue(borEq(borVec3Y(&w), 2. * 5.));
    assertTrue(borEq(borVec3Z(&w), 2. * 2.));

    borVec3Set(&w, 1.2, -2.1, 0.15);
    borMat4SetTranslate(&tr, &w);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), 1.2 + -1.2));
    assertTrue(borEq(borVec3Y(&w), -2.1 + 5.));
    assertTrue(borEq(borVec3Z(&w), 2. + 0.15));

    borVec3Set(&v, 1., 1., 0.);
    borVec3Set(&axis, 0., 0., 1.);
    borMat4SetRot(&tr, M_PI_4, &axis);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), BOR_ZERO));
    assertTrue(borEq(borVec3Y(&w), BOR_SQRT(2.)));
    assertTrue(borEq(borVec3Z(&w), BOR_ZERO));

    borVec3Set(&v, 1., 0., 1.);
    borVec3Set(&axis, 0., 1., 0.);
    borMat4SetRot(&tr, M_PI_4, &axis);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), BOR_SQRT(2.)));
    assertTrue(borEq(borVec3Y(&w), BOR_ZERO));
    assertTrue(borEq(borVec3Z(&w), BOR_ZERO));


    // ------

    borVec3Set(&v, 1., 2., 3.);
    borMat4SetIdentity(&tr);
    borVec3Set(&w, 0.5, 0.7, 0.1);
    borMat4Translate(&tr, &w);
    borMat4TrScale(&tr, 2.);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), 2. * (0.5 + 1.)));
    assertTrue(borEq(borVec3Y(&w), 2. * (0.7 + 2.)));
    assertTrue(borEq(borVec3Z(&w), 2. * (0.1 + 3.)));

    borVec3Set(&v, .5, .3, .9);
    borVec3Set(&axis, 1, 1, 1);
    borMat4Rot(&tr, M_PI_4, &axis);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), 2.));
    assertTrue(borEq(borVec3Y(&w), 2.));
    assertTrue(borEq(borVec3Z(&w), 2.));

    borVec3Set(&axis, 0, 0, 1);
    borMat4Rot(&tr, M_PI_4, &axis);
    borMat4MulVec3(&w, &tr, &v);
    assertTrue(borEq(borVec3X(&w), 0.));
    assertTrue(borEq(borVec3Y(&w), 2. * BOR_SQRT(2)));
    assertTrue(borEq(borVec3Z(&w), 2.));
#endif /* BOR_SSE_SINGLE */
}
