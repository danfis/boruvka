#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/mat3.h>
#include <boruvka/dbg.h>
#include "data.h"

TEST(mat3SetUp)
{
}

TEST(mat3TearDown)
{
}

TEST(mat3Alloc)
{
    /*
    bor_mat3_t *v, w;

    v = borMat3New(0., 1., 2., 3.);
    borMat3Set(&w, 0., 1., 2., 3.);
    assertTrue(borMat3Eq(v, &w));
    borMat3Del(v);
    */
}

TEST(mat3Tr)
{
#ifndef BOR_SSE_SINGLE
    bor_vec2_t v, w;
    bor_mat3_t tr;

    borVec2Set(&v, -1.2, 5);

    borMat3SetIdentity(&tr);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borVec2Eq(&w, &v));

    borMat3SetScale(&tr, 2.);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), 2. * -1.2));
    assertTrue(borEq(borVec2Y(&w), 2. * 5.));

    borVec2Set(&w, 1.2, -2.1);
    borMat3SetTranslate(&tr, &w);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), 1.2 + -1.2));
    assertTrue(borEq(borVec2Y(&w), -2.1 + 5.));

    borVec2Set(&v, 1., 1.);
    borMat3SetRot(&tr, M_PI_4);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), BOR_ZERO));
    assertTrue(borEq(borVec2Y(&w), BOR_SQRT(2.)));


    // ------

    borVec2Set(&v, 1., 2.);
    borMat3SetIdentity(&tr);
    borVec2Set(&w, 0.5, 0.7);
    borMat3Translate(&tr, &w);
    borMat3TrScale(&tr, 2.);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), 2. * (0.5 + 1.)));
    assertTrue(borEq(borVec2Y(&w), 2. * (0.7 + 2.)));

    borVec2Set(&v, .5, .3);
    borMat3Rot(&tr, M_PI_4);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), BOR_ZERO));
    assertTrue(borEq(borVec2Y(&w), 2. * BOR_SQRT(2.)));


    borVec2Set(&v, .5, .3);
    borMat3SetIdentity(&tr);
    borVec2Set(&w, 0.5, 0.7);
    borMat3Translate(&tr, &w);
    borMat3TrScale(&tr, 2.);
    borMat3Rot(&tr, -M_PI_4);
    borMat3MulVec2(&w, &tr, &v);
    assertTrue(borEq(borVec2X(&w), 2 * BOR_SQRT(2.)));
    assertTrue(borEq(borVec2Y(&w), BOR_ZERO));

    /*
    DBG("%g", borMat3Get(&tr, 0, 0));
    DBG("%g", borMat3DotRow(&tr, 0, -1.2, 5, 1));
    DBG("%g", borMat3DotRow(&tr, 1, -1.2, 5, 1));
    DBG("%g", borMat3DotRow(&tr, 2, -1.2, 5, 1));

    DBG("w: %.10g %.10g %.10g", borVec2X(&w), borVec2Y(&w), BOR_EPS);
    */
#endif /* BOR_SSE_SINGLE */
}
