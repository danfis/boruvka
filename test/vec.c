#include <stdio.h>
#include "cu.h"

#include <boruvka/vec.h>

TEST(vecSetUp)
{
}

TEST(vecTearDown)
{
}

TEST(vecInit)
{
    bor_vec_t *vec;

    vec = borVecNew(34);
    borVecDel(vec);
}

static void prVec(size_t len, const char *prefix, const bor_vec_t *v)
{
    size_t i;

    fprintf(stdout, prefix);

    fprintf(stdout, "[");
    for (i = 0; i < len; i++){
        fprintf(stdout, " %g", borVecGet(v, i));
    }
    fprintf(stdout, " ]\n");
}

TEST(vecOperators)
{
    BOR_VEC(u, 6);
    BOR_VEC(v, 6);
    BOR_VEC(w, 6);

    printf("---- vecOperators ---\n");

    borVecSetAll(6, v, 1.);
    prVec(6, "v: ", v);
    borVecSetZero(6, v);
    prVec(6, "v: ", v);
    printf("\n");

    borVecSet(v, 0, 1.);
    borVecSet(v, 1, 2.);
    borVecSet(v, 2, 1.4);
    borVecSet(v, 3, -.6);
    borVecSet(v, 4, 8.3);
    borVecSet(v, 5, 11.);
    prVec(6, "v: ", v);
    assertTrue(borEq(borVecLen2(6, v), 197.21));

    borVecSet(w, 0, 3.);
    borVecSet(w, 1, 1.);
    borVecSet(w, 2, 9.4);
    borVecSet(w, 3, -3.3);
    borVecSet(w, 4, 1.2);
    borVecSet(w, 5, 14.);
    prVec(6, "w: ", w);


    borVecAdd2(6, u, v, w);
    prVec(6, "u = v + w ", u);
    borVecAdd(6, u, v);
    prVec(6, "u += v    ", u);
    borVecSub2(6, u, v, w);
    prVec(6, "u = v - w ", u);
    borVecSub(6, u, v);
    prVec(6, "u -= v    ", u);
    borVecAddConst2(6, u, v, 10.);
    prVec(6, "u = v + 10 ", u);
    borVecAddConst(6, u, 100.);
    prVec(6, "u += 100   ", u);
    borVecSubConst2(6, u, v, 10.);
    prVec(6, "u = v - 10 ", u);
    borVecSubConst(6, u, 100.);
    prVec(6, "u -= 100   ", u);

    borVecScale(6, u, 2.);
    prVec(6, "u *= 2    ", u);
    printf("\n");

    borVecMulComp2(6, u, v, w);
    prVec(6, "u = v .* w ", u);
    borVecMulComp(6, u, w);
    prVec(6, "u .*= w    ", u);
}
