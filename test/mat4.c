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

static void pr(const bor_mat4_t *m, const char *p)
{
    printf("# %s %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
           p,
           borMat4Get(m, 0, 0), borMat4Get(m, 0, 1), borMat4Get(m, 0, 2), borMat4Get(m, 0, 3),
           borMat4Get(m, 1, 0), borMat4Get(m, 1, 1), borMat4Get(m, 1, 2), borMat4Get(m, 1, 3), 
           borMat4Get(m, 2, 0), borMat4Get(m, 2, 1), borMat4Get(m, 2, 2), borMat4Get(m, 2, 3),
           borMat4Get(m, 3, 0), borMat4Get(m, 3, 1), borMat4Get(m, 3, 2), borMat4Get(m, 3, 3));
}

TEST(mat4AddSub)
{
    size_t i;
    bor_mat4_t m;

    printf("# ---- add sub ----\n");
    for (i = 0; i < mat4s_len - 1; i++){
        borMat4Add2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "add");
        borMat4Copy(&m, &mat4s[i]);
        borMat4Add(&m, &mat4s[i + 1]);
        pr(&m, "add");

        borMat4Sub2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "sub");
        borMat4Copy(&m, &mat4s[i]);
        borMat4Sub(&m, &mat4s[i + 1]);
        pr(&m, "sub");
    }
    printf("# ---- add sub end ----\n\n");
}

TEST(mat4Const)
{
    size_t i;
    bor_mat4_t m;
    bor_real_t c;

    printf("# ---- const ----\n");
    for (i = 0; i < mat4s_len; i++){
        c = borMat4Get(&mat4s[i], 0, 0);
        borMat4Scale2(&m, &mat4s[i], c);
        pr(&m, "scale");

        borMat4Copy(&m, &mat4s[i]);
        borMat4Scale(&m, c);
        pr(&m, "scale");

        borMat4AddConst2(&m, &mat4s[i], c);
        pr(&m, "add");
        borMat4Copy(&m, &mat4s[i]);
        borMat4AddConst(&m, c);
        pr(&m, "add");
    }
    printf("# ---- const end ----\n\n");
}

TEST(mat4Mul)
{
    size_t i;
    bor_mat4_t m;

    printf("# ---- mul ----\n");
    for (i = 0; i < mat4s_len - 1; i++){
        borMat4Mul2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "right");
        borMat4Copy(&m, &mat4s[i]);
        borMat4MulLeft(&m, &mat4s[i + 1]);
        pr(&m, "left");
    }
    printf("# ---- mul end ----\n\n");
}

TEST(mat4Trans)
{
    size_t i;
    bor_mat4_t m;

    printf("# ---- trans ----\n");
    for (i = 0; i < mat4s_len; i++){
        borMat4Trans2(&m, &mat4s[i]);
        pr(&m, "");
        borMat4Copy(&m, &mat4s[i]);
        borMat4Trans(&m);
        pr(&m, "");
    }
    printf("# ---- trans end ----\n\n");
}

TEST(mat4Det)
{
    size_t i;
    bor_real_t d;

    printf("# ---- det ----\n");
    for (i = 0; i < mat4s_len; i++){
        d = borMat4Det(&mat4s[i]);
        printf("# %g\n", d);
    }
    printf("# ---- det end ----\n\n");
}

TEST(mat4Inv)
{
    size_t i;
    bor_mat4_t m;

    printf("# ---- inv ----\n");
    for (i = 0; i < mat4s_len; i++){
        if (borMat4Inv2(&m, &mat4s[i]) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }

        borMat4Copy(&m, &mat4s[i]);
        if (borMat4Inv(&m) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }
    }
    printf("# ---- inv end ----\n\n");
}

TEST(mat4MulVec)
{
    size_t i;
    bor_vec4_t u, v;
    bor_vec3_t a, b;

    printf("# ---- mul vec ----\n");
    for (i = 0; i < mat4s_len; i++){
        borVec4Set(&v, borMat4Get(&mat4s[i], 0, 1),
                       borMat4Get(&mat4s[i], 0, 2),
                       borMat4Get(&mat4s[i], 0, 3),
                       borMat4Get(&mat4s[i], 1, 0));
        borVec3Set(&b, borMat4Get(&mat4s[i], 0, 3),
                       borMat4Get(&mat4s[i], 1, 0),
                       borMat4Get(&mat4s[i], 1, 1));

        borMat4MulVec(&u, &mat4s[i], &v);
        borMat4MulVec3(&a, &mat4s[i], &b);

        printf("# vec4 %g %g %g %g\n",
               borVec4X(&u), borVec4Y(&u), borVec4Z(&u), borVec4W(&u));
        printf("# vec3 %g %g %g\n",
               borVec3X(&a), borVec3Y(&a), borVec3Z(&a));
    }
    printf("# ---- mul vec end ----\n\n");
}

TEST(mat4Tr)
{
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
}
