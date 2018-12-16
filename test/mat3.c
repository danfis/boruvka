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

static void pr(const bor_mat3_t *m, const char *p)
{
    printf("# %s %g %g %g %g %g %g %g %g %g\n",
           p,
           borMat3Get(m, 0, 0), borMat3Get(m, 0, 1), borMat3Get(m, 0, 2), 
           borMat3Get(m, 1, 0), borMat3Get(m, 1, 1), borMat3Get(m, 1, 2), 
           borMat3Get(m, 2, 0), borMat3Get(m, 2, 1), borMat3Get(m, 2, 2));
}

TEST(mat3AddSub)
{
    size_t i;
    bor_mat3_t m;

    printf("# ---- add sub ----\n");
    for (i = 0; i < mat3s_len - 1; i++){
        borMat3Add2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "add");
        borMat3Copy(&m, &mat3s[i]);
        borMat3Add(&m, &mat3s[i + 1]);
        pr(&m, "add");

        borMat3Sub2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "sub");
        borMat3Copy(&m, &mat3s[i]);
        borMat3Sub(&m, &mat3s[i + 1]);
        pr(&m, "sub");
    }
    printf("# ---- add sub end ----\n\n");
}

TEST(mat3Const)
{
    size_t i;
    bor_mat3_t m;
    bor_real_t c;

    printf("# ---- const ----\n");
    for (i = 0; i < mat3s_len; i++){
        c = borMat3Get(&mat3s[i], 0, 0);
        borMat3Scale2(&m, &mat3s[i], c);
        pr(&m, "scale");

        borMat3Copy(&m, &mat3s[i]);
        borMat3Scale(&m, c);
        pr(&m, "scale");

        borMat3AddConst2(&m, &mat3s[i], c);
        pr(&m, "add");
        borMat3Copy(&m, &mat3s[i]);
        borMat3AddConst(&m, c);
        pr(&m, "add");
    }
    printf("# ---- const end ----\n\n");
}

TEST(mat3Mul)
{
    size_t i;
    bor_mat3_t m;

    printf("# ---- mul ----\n");
    for (i = 0; i < mat3s_len - 1; i++){
        borMat3Mul2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "right");
        borMat3Copy(&m, &mat3s[i]);
        borMat3MulLeft(&m, &mat3s[i + 1]);
        pr(&m, "left");
    }
    printf("# ---- mul end ----\n\n");
}

TEST(mat3Trans)
{
    size_t i;
    bor_mat3_t m;

    printf("# ---- trans ----\n");
    for (i = 0; i < mat3s_len; i++){
        borMat3Trans2(&m, &mat3s[i]);
        pr(&m, "");
        borMat3Copy(&m, &mat3s[i]);
        borMat3Trans(&m);
        pr(&m, "");
    }
    printf("# ---- trans end ----\n\n");
}

TEST(mat3Det)
{
    size_t i;
    bor_real_t d;

    printf("# ---- det ----\n");
    for (i = 0; i < mat3s_len; i++){
        d = borMat3Det(&mat3s[i]);
        printf("# %g\n", d);
    }
    printf("# ---- det end ----\n\n");
}

TEST(mat3Inv)
{
    size_t i;
    bor_mat3_t m;

    printf("# ---- inv ----\n");
    for (i = 0; i < mat3s_len; i++){
        if (borMat3Inv2(&m, &mat3s[i]) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }

        borMat3Copy(&m, &mat3s[i]);
        if (borMat3Inv(&m) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }
    }
    printf("# ---- inv end ----\n\n");
}

TEST(mat3MulVec)
{
    size_t i;
    bor_vec3_t u, v;
    bor_vec2_t a, b;

    printf("# ---- mul vec ----\n");
    for (i = 0; i < mat3s_len; i++){
        borVec3Set(&v, borMat3Get(&mat3s[i], 0, 1),
                       borMat3Get(&mat3s[i], 0, 2),
                       borMat3Get(&mat3s[i], 1, 0));
        borVec2Set(&b, borMat3Get(&mat3s[i], 1, 0),
                       borMat3Get(&mat3s[i], 1, 1));

        borMat3MulVec(&u, &mat3s[i], &v);
        borMat3MulVec2(&a, &mat3s[i], &b);

        printf("# vec3 %g %g %g\n",
               borVec3X(&u), borVec3Y(&u), borVec3Z(&u));
        printf("# vec2 %g %g\n",
               borVec2X(&a), borVec2Y(&a));
    }
    printf("# ---- mul vec end ----\n\n");
}

TEST(mat3Tr)
{
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
}
