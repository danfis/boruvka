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

    v = ferVec4New(0., 1., 2., 3.);
    ferVec4Set(&w, 0., 1., 2., 3.);
    assertTrue(ferVec4Eq(v, &w));
    ferVec4Del(v);
}

TEST(vec4Add)
{
    bor_vec4_t v;
    size_t i;

    printf("# ---- add ----\n");
    ferVec4Set(&v, BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs4_len; i++){
        ferVec4Add(&v, &vecs4[i]);
        printf("# %g %g %g %g\n", ferVec4X(&v), ferVec4Y(&v), ferVec4Z(&v), ferVec4W(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec4Sub)
{
    bor_vec4_t v;
    size_t i;

    printf("# ---- sub ----\n");
    ferVec4Set(&v, BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs4_len; i++){
        ferVec4Sub(&v, &vecs4[i]);
        printf("# %g %g %g %g\n", ferVec4X(&v), ferVec4Y(&v), ferVec4Z(&v), ferVec4W(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec4Scale)
{
    bor_vec4_t v;
    size_t i;

    printf("# ---- scale ----\n");
    for (i = 0; i < vecs4_len; i++){
        ferVec4Copy(&v, &vecs4[i]);
        ferVec4Scale(&v, ferVec4X(&vecs4[i]));
        printf("# %g %g %g %g\n", ferVec4X(&v), ferVec4Y(&v), ferVec4Z(&v), ferVec4W(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec4Normalize)
{
    bor_vec4_t v;
    size_t i;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs4_len; i++){
        ferVec4Copy(&v, &vecs4[i]);
        ferVec4Normalize(&v);
        printf("# %g %g %g %g\n", ferVec4X(&v), ferVec4Y(&v), ferVec4Z(&v), ferVec4W(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec4Dot)
{
    bor_real_t dot;
    size_t i;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        dot = ferVec4Dot(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec4Mul)
{
    bor_vec4_t v;
    size_t i;

    printf("# ---- mul ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        ferVec4MulComp2(&v, &vecs4[i], &vecs4[i + 1]);
        printf("# %g %g %g %g\n", ferVec4X(&v), ferVec4Y(&v), ferVec4Z(&v), ferVec4W(&v));
    }
    printf("# ---- mul end ----\n\n");
}


TEST(vec4Len2)
{
    bor_real_t d;
    size_t i;

    printf("# ---- len2 ----\n");
    for (i = 0; i < vecs4_len; i++){
        d = ferVec4Len2(&vecs4[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(vec4Len)
{
    bor_real_t d;
    size_t i;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs4_len; i++){
        d = ferVec4Len(&vecs4[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec4Dist2)
{
    bor_real_t d;
    size_t i;

    printf("# ---- dist2 ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        d = ferVec4Dist2(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist2 end ----\n\n");
}

TEST(vec4Dist)
{
    bor_real_t d;
    size_t i;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        d = ferVec4Dist(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist end ----\n\n");
}

