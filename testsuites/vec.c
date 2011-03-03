#include <stdio.h>
#include "cu.h"

#include <fermat/vec.h>

TEST(vecSetUp)
{
}

TEST(vecTearDown)
{
}

TEST(vecInit)
{
    fer_vec_t *vec;

    vec = ferVecNew(34);
    ferVecDel(vec);
}

static void prVec(size_t len, const char *prefix, const fer_vec_t *v)
{
    size_t i;

    fprintf(stdout, prefix);

    fprintf(stdout, "[");
    for (i = 0; i < len; i++){
        fprintf(stdout, " %g", ferVecGet(v, i));
    }
    fprintf(stdout, " ]\n");
}

TEST(vecOperators)
{
    FER_VEC(u, 6);
    FER_VEC(v, 6);
    FER_VEC(w, 6);

    printf("---- vecOperators ---\n");

    ferVecSetAll(6, v, 1.);
    prVec(6, "v: ", v);
    ferVecSetZero(6, v);
    prVec(6, "v: ", v);
    printf("\n");

    ferVecSet(v, 0, 1.);
    ferVecSet(v, 1, 2.);
    ferVecSet(v, 2, 1.4);
    ferVecSet(v, 3, -.6);
    ferVecSet(v, 4, 8.3);
    ferVecSet(v, 5, 11.);
    prVec(6, "v: ", v);
    assertTrue(ferEq(ferVecLen2(6, v), 197.21));

    ferVecSet(w, 0, 3.);
    ferVecSet(w, 1, 1.);
    ferVecSet(w, 2, 9.4);
    ferVecSet(w, 3, -3.3);
    ferVecSet(w, 4, 1.2);
    ferVecSet(w, 5, 14.);
    prVec(6, "w: ", w);


    ferVecAdd2(6, u, v, w);
    prVec(6, "u = v + w ", u);
    ferVecAdd(6, u, v);
    prVec(6, "u += v    ", u);
    ferVecSub2(6, u, v, w);
    prVec(6, "u = v - w ", u);
    ferVecSub(6, u, v);
    prVec(6, "u -= v    ", u);
    ferVecAddConst2(6, u, v, 10.);
    prVec(6, "u = v + 10 ", u);
    ferVecAddConst(6, u, 100.);
    prVec(6, "u += 100   ", u);
    ferVecSubConst2(6, u, v, 10.);
    prVec(6, "u = v - 10 ", u);
    ferVecSubConst(6, u, 100.);
    prVec(6, "u -= 100   ", u);

    ferVecScale(6, u, 2.);
    prVec(6, "u *= 2    ", u);
    printf("\n");

    ferVecMulComp2(6, u, v, w);
    prVec(6, "u = v .* w ", u);
    ferVecMulComp(6, u, w);
    prVec(6, "u .*= w    ", u);
}
