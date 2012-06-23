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

    v = ferMat4New(0., 1., 2., 3.);
    ferMat4Set(&w, 0., 1., 2., 3.);
    assertTrue(ferMat4Eq(v, &w));
    ferMat4Del(v);
    */
}

static void pr(const bor_mat4_t *m, const char *p)
{
    printf("# %s %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
           p,
           ferMat4Get(m, 0, 0), ferMat4Get(m, 0, 1), ferMat4Get(m, 0, 2), ferMat4Get(m, 0, 3),
           ferMat4Get(m, 1, 0), ferMat4Get(m, 1, 1), ferMat4Get(m, 1, 2), ferMat4Get(m, 1, 3), 
           ferMat4Get(m, 2, 0), ferMat4Get(m, 2, 1), ferMat4Get(m, 2, 2), ferMat4Get(m, 2, 3),
           ferMat4Get(m, 3, 0), ferMat4Get(m, 3, 1), ferMat4Get(m, 3, 2), ferMat4Get(m, 3, 3));
}

TEST(mat4AddSub)
{
    size_t i;
    bor_mat4_t m;

    printf("# ---- add sub ----\n");
    for (i = 0; i < mat4s_len - 1; i++){
        ferMat4Add2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "add");
        ferMat4Copy(&m, &mat4s[i]);
        ferMat4Add(&m, &mat4s[i + 1]);
        pr(&m, "add");

        ferMat4Sub2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "sub");
        ferMat4Copy(&m, &mat4s[i]);
        ferMat4Sub(&m, &mat4s[i + 1]);
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
        c = ferMat4Get(&mat4s[i], 0, 0);
        ferMat4Scale2(&m, &mat4s[i], c);
        pr(&m, "scale");

        ferMat4Copy(&m, &mat4s[i]);
        ferMat4Scale(&m, c);
        pr(&m, "scale");

        ferMat4AddConst2(&m, &mat4s[i], c);
        pr(&m, "add");
        ferMat4Copy(&m, &mat4s[i]);
        ferMat4AddConst(&m, c);
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
        ferMat4Mul2(&m, &mat4s[i], &mat4s[i + 1]);
        pr(&m, "right");
        ferMat4Copy(&m, &mat4s[i]);
        ferMat4MulLeft(&m, &mat4s[i + 1]);
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
        ferMat4Trans2(&m, &mat4s[i]);
        pr(&m, "");
        ferMat4Copy(&m, &mat4s[i]);
        ferMat4Trans(&m);
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
        d = ferMat4Det(&mat4s[i]);
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
        if (ferMat4Inv2(&m, &mat4s[i]) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }

        ferMat4Copy(&m, &mat4s[i]);
        if (ferMat4Inv(&m) == 0){
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
        ferVec4Set(&v, ferMat4Get(&mat4s[i], 0, 1),
                       ferMat4Get(&mat4s[i], 0, 2),
                       ferMat4Get(&mat4s[i], 0, 3),
                       ferMat4Get(&mat4s[i], 1, 0));
        ferVec3Set(&b, ferMat4Get(&mat4s[i], 0, 3),
                       ferMat4Get(&mat4s[i], 1, 0),
                       ferMat4Get(&mat4s[i], 1, 1));

        ferMat4MulVec(&u, &mat4s[i], &v);
        ferMat4MulVec3(&a, &mat4s[i], &b);

        printf("# vec4 %g %g %g %g\n",
               ferVec4X(&u), ferVec4Y(&u), ferVec4Z(&u), ferVec4W(&u));
        printf("# vec3 %g %g %g\n",
               ferVec3X(&a), ferVec3Y(&a), ferVec3Z(&a));
    }
    printf("# ---- mul vec end ----\n\n");
}

TEST(mat4Tr)
{
    bor_vec3_t v, w, axis;
    bor_mat4_t tr;

    ferVec3Set(&v, -1.2, 5, 2);

    ferMat4SetIdentity(&tr);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferVec3Eq(&w, &v));

    ferMat4SetScale(&tr, 2.);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), 2. * -1.2));
    assertTrue(ferEq(ferVec3Y(&w), 2. * 5.));
    assertTrue(ferEq(ferVec3Z(&w), 2. * 2.));

    ferVec3Set(&w, 1.2, -2.1, 0.15);
    ferMat4SetTranslate(&tr, &w);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), 1.2 + -1.2));
    assertTrue(ferEq(ferVec3Y(&w), -2.1 + 5.));
    assertTrue(ferEq(ferVec3Z(&w), 2. + 0.15));

    ferVec3Set(&v, 1., 1., 0.);
    ferVec3Set(&axis, 0., 0., 1.);
    ferMat4SetRot(&tr, M_PI_4, &axis);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), BOR_ZERO));
    assertTrue(ferEq(ferVec3Y(&w), BOR_SQRT(2.)));
    assertTrue(ferEq(ferVec3Z(&w), BOR_ZERO));

    ferVec3Set(&v, 1., 0., 1.);
    ferVec3Set(&axis, 0., 1., 0.);
    ferMat4SetRot(&tr, M_PI_4, &axis);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), BOR_SQRT(2.)));
    assertTrue(ferEq(ferVec3Y(&w), BOR_ZERO));
    assertTrue(ferEq(ferVec3Z(&w), BOR_ZERO));


    // ------

    ferVec3Set(&v, 1., 2., 3.);
    ferMat4SetIdentity(&tr);
    ferVec3Set(&w, 0.5, 0.7, 0.1);
    ferMat4Translate(&tr, &w);
    ferMat4TrScale(&tr, 2.);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), 2. * (0.5 + 1.)));
    assertTrue(ferEq(ferVec3Y(&w), 2. * (0.7 + 2.)));
    assertTrue(ferEq(ferVec3Z(&w), 2. * (0.1 + 3.)));

    ferVec3Set(&v, .5, .3, .9);
    ferVec3Set(&axis, 1, 1, 1);
    ferMat4Rot(&tr, M_PI_4, &axis);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), 2.));
    assertTrue(ferEq(ferVec3Y(&w), 2.));
    assertTrue(ferEq(ferVec3Z(&w), 2.));

    ferVec3Set(&axis, 0, 0, 1);
    ferMat4Rot(&tr, M_PI_4, &axis);
    ferMat4MulVec3(&w, &tr, &v);
    assertTrue(ferEq(ferVec3X(&w), 0.));
    assertTrue(ferEq(ferVec3Y(&w), 2. * BOR_SQRT(2)));
    assertTrue(ferEq(ferVec3Z(&w), 2.));
}
