#include <stdio.h>
#include <string.h>
#include "cu.h"

#include <fermat/mat.h>

TEST(matSetUp)
{
}

TEST(matTearDown)
{
}

TEST(matInit)
{
    fer_mat_t mat;
    fer_mat_t *mat2;

    ferMatInit(&mat, 32, 12);
    assertEquals(ferMatRows(&mat), 32);
    assertEquals(ferMatCols(&mat), 12);
    ferMatDestroy(&mat);

    mat2 = ferMatNew(34, 3);
    assertEquals(ferMatRows(mat2), 34);
    assertEquals(ferMatCols(mat2), 3);
    ferMatDel(mat2);
}

static void prMat(const char *prefix, const fer_mat_t *v)
{
    size_t i, j, k, rows, cols, slen;

    slen = strlen(prefix);

    rows = ferMatRows(v);
    cols = ferMatCols(v);

    fprintf(stdout, prefix);
    fprintf(stdout, "[");
    for (i = 0; i < rows; i++){
        if (i != 0){
            for (k = 0; k < slen; k++){
                fprintf(stdout, " ");
            }
            fprintf(stdout, " ");
        }

        for (j = 0; j < cols; j++){
            fprintf(stdout, " %g", ferMatGet(v, i, j));
        }

        if (i < rows - 1){
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, " ]\n");
}

TEST(matOperators)
{
    fer_mat_t u, v, w, q, s, s2;

    printf("---- matOperators ---\n");

    ferMatInit(&u, 2, 3);
    ferMatInit(&v, 2, 3);
    ferMatInit(&w, 2, 3);
    ferMatInit(&q, 3, 2);
    ferMatInit(&s, 3, 3);
    ferMatInit(&s2, 3, 3);

    ferMatSetAll(&v, 1.);
    prMat("v: ", &v);
    ferMatSetZero(&v);
    prMat("v: ", &v);
    ferMatSetIdentity(&v);
    prMat("v: ", &v);
    ferMatSetDiag(&v, 2.);
    prMat("v: ", &v);
    printf("\n");

    ferMatSet(&v, 0, 0, 1.);
    ferMatSet(&v, 0, 1, 1.1);
    ferMatSet(&v, 0, 2, 3.4);
    ferMatSet(&v, 1, 0, 0.9);
    ferMatSet(&v, 1, 1, 8.2);
    ferMatSet(&v, 1, 2, 3.7);
    prMat("v: ", &v);

    ferMatSet(&w, 0, 0, 2.);
    ferMatSet(&w, 0, 1, 3.);
    ferMatSet(&w, 0, 2, 4.4);
    ferMatSet(&w, 1, 0, 7.);
    ferMatSet(&w, 1, 1, 8.);
    ferMatSet(&w, 1, 2, 7.7);
    prMat("w: ", &w);

    assertNotEquals(ferMatAdd(&v, &q), 0);
    assertEquals(ferMatAdd2(&u, &v, &w), 0);
    prMat("u = v + w ", &u);
    assertEquals(ferMatAdd(&u, &v), 0);
    prMat("u += v    ", &u);
    assertEquals(ferMatSub2(&u, &v, &w), 0);
    prMat("u = v - w ", &u);
    assertEquals(ferMatSub(&u, &v), 0);
    prMat("u -= v    ", &u);
    assertEquals(ferMatAddConst2(&u, &v, 10.), 0);
    prMat("u = v + 10 ", &u);
    ferMatAddConst(&u, 100.);
    prMat("u += 100   ", &u);
    assertEquals(ferMatSubConst2(&u, &v, 10.), 0);
    prMat("u = v - 10 ", &u);
    ferMatSubConst(&u, 100.);
    prMat("u -= 100   ", &u);

    ferMatScale(&u, 2.);
    prMat("u *= 2    ", &u);
    assertEquals(ferMatScale2(&u, &v, 2.), 0);
    prMat("u = 2 * v ", &u);
    printf("\n");

    assertEquals(ferMatMulComp2(&u, &v, &w), 0);
    prMat("u = v .* w ", &u);
    assertEquals(ferMatMulComp(&u, &w), 0);
    prMat("u .*= w    ", &u);
    printf("\n");

    assertEquals(ferMatTrans2(&q, &w), 0);
    prMat("q = w^t ", &q);
    assertEquals(ferMatMul2(&s, &q, &v), 0);
    prMat("s = q * v", &s);
    ferMatTrans(&s);
    prMat("s = s^t ", &s);
    printf("\n");

    ferMatSet(&s, 0, 1, 4.4);
    ferMatSet(&s, 1, 2, 1.4);
    ferMatSet(&s, 2, 2, 2.4);
    prMat("s ", &s);
    printf("det: %g\n", ferMatDet(&s));
    printf("regular: %d\n", ferMatRegular(&s));
    printf("singular: %d\n", ferMatSingular(&s));

    ferMatInv2(&s2, &s);
    prMat("s2 = inv(s) ", &s2);
    ferMatInv(&s2);
    prMat("s2 = inv(s2) ", &s2);

    ferMatDestroy(&u);
    ferMatDestroy(&v);
    ferMatDestroy(&w);
    ferMatDestroy(&q);
    ferMatDestroy(&s);
    ferMatDestroy(&s2);
}
