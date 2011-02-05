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
    fer_vec_t vec;
    fer_vec_t *vec2;

    ferVecInit(&vec, 32);
    assertEquals(ferVecSize(&vec), 32);
    ferVecDestroy(&vec);

    vec2 = ferVecNew(34);
    assertEquals(ferVecSize(vec2), 34);
    ferVecDel(vec2);
}

static void prVec(const char *prefix, const fer_vec_t *v)
{
    size_t i, len;

    fprintf(stdout, prefix);

    len = ferVecSize(v);
    fprintf(stdout, "[");
    for (i = 0; i < len; i++){
        fprintf(stdout, " %g", ferVecGet(v, i));
    }
    fprintf(stdout, " ]\n");
}

TEST(vecOperators)
{
    fer_vec_t u, v, w, q;

    printf("---- vecOperators ---\n");

    ferVecInit(&u, 6);
    ferVecInit(&v, 6);
    ferVecInit(&w, 6);
    ferVecInit(&q, 1);

    ferVecSetAll(&v, 1.);
    prVec("v: ", &v);
    ferVecSetZero(&v);
    prVec("v: ", &v);
    printf("\n");

    ferVecSet(&v, 0, 1.);
    ferVecSet(&v, 1, 2.);
    ferVecSet(&v, 2, 1.4);
    ferVecSet(&v, 3, -.6);
    ferVecSet(&v, 4, 8.3);
    ferVecSet(&v, 5, 11.);
    prVec("v: ", &v);
    assertTrue(ferEq(ferVecLen2(&v), 197.21));

    ferVecSet(&w, 0, 3.);
    ferVecSet(&w, 1, 1.);
    ferVecSet(&w, 2, 9.4);
    ferVecSet(&w, 3, -3.3);
    ferVecSet(&w, 4, 1.2);
    ferVecSet(&w, 5, 14.);
    prVec("w: ", &w);


    assertNotEquals(ferVecAdd(&v, &q), 0);
    assertEquals(ferVecAdd2(&u, &v, &w), 0);
    prVec("u = v + w ", &u);
    assertEquals(ferVecAdd(&u, &v), 0);
    prVec("u += v    ", &u);
    assertEquals(ferVecSub2(&u, &v, &w), 0);
    prVec("u = v - w ", &u);
    assertEquals(ferVecSub(&u, &v), 0);
    prVec("u -= v    ", &u);
    assertEquals(ferVecAddConst2(&u, &v, 10.), 0);
    prVec("u = v + 10 ", &u);
    ferVecAddConst(&u, 100.);
    prVec("u += 100   ", &u);
    assertEquals(ferVecSubConst2(&u, &v, 10.), 0);
    prVec("u = v - 10 ", &u);
    ferVecSubConst(&u, 100.);
    prVec("u -= 100   ", &u);

    ferVecScale(&u, 2.);
    prVec("u *= 2    ", &u);
    printf("\n");

    assertEquals(ferVecMulComp2(&u, &v, &w), 0);
    prVec("u = v .* w ", &u);
    assertEquals(ferVecMulComp(&u, &w), 0);
    prVec("u .*= w    ", &u);


    ferVecDestroy(&u);
    ferVecDestroy(&v);
    ferVecDestroy(&w);
    ferVecDestroy(&q);
}
