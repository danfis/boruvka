#include <stdio.h>
#include <cu/cu.h>
#include <mg/vec4.h>
#include <mg/dbg.h>
#include "data.h"

TEST(vec4SetUp)
{
}

TEST(vec4TearDown)
{
}

TEST(vec4Alloc)
{
    mg_vec4_t *v, w;

    v = mgVec4New(0., 1., 2., 3.);
    mgVec4Set(&w, 0., 1., 2., 3.);
    assertTrue(mgVec4Eq(v, &w));
    mgVec4Del(v);
}

TEST(vec4Add)
{
    mg_vec4_t v;
    size_t i;

    printf("# ---- add ----\n");
    mgVec4Set(&v, MG_ZERO, MG_ZERO, MG_ZERO, MG_ZERO);
    for (i = 0; i < vecs4_len; i++){
        mgVec4Add(&v, &vecs4[i]);
        printf("# %g %g %g %g\n", mgVec4X(&v), mgVec4Y(&v), mgVec4Z(&v), mgVec4W(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec4Sub)
{
    mg_vec4_t v;
    size_t i;

    printf("# ---- sub ----\n");
    mgVec4Set(&v, MG_ZERO, MG_ZERO, MG_ZERO, MG_ZERO);
    for (i = 0; i < vecs4_len; i++){
        mgVec4Sub(&v, &vecs4[i]);
        printf("# %g %g %g %g\n", mgVec4X(&v), mgVec4Y(&v), mgVec4Z(&v), mgVec4W(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec4Scale)
{
    mg_vec4_t v;
    size_t i;

    printf("# ---- scale ----\n");
    for (i = 0; i < vecs4_len; i++){
        mgVec4Copy(&v, &vecs4[i]);
        mgVec4Scale(&v, mgVec4X(&vecs4[i]));
        printf("# %g %g %g %g\n", mgVec4X(&v), mgVec4Y(&v), mgVec4Z(&v), mgVec4W(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec4Normalize)
{
    mg_vec4_t v;
    size_t i;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs4_len; i++){
        mgVec4Copy(&v, &vecs4[i]);
        mgVec4Normalize(&v);
        printf("# %g %g %g %g\n", mgVec4X(&v), mgVec4Y(&v), mgVec4Z(&v), mgVec4W(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec4Dot)
{
    mg_real_t dot;
    size_t i;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        dot = mgVec4Dot(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec4Mul)
{
    mg_vec4_t v;
    size_t i;

    printf("# ---- mul ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        mgVec4Mul2(&v, &vecs4[i], &vecs4[i + 1]);
        printf("# %g %g %g %g\n", mgVec4X(&v), mgVec4Y(&v), mgVec4Z(&v), mgVec4W(&v));
    }
    printf("# ---- mul end ----\n\n");
}


TEST(vec4Len2)
{
    mg_real_t d;
    size_t i;

    printf("# ---- len2 ----\n");
    for (i = 0; i < vecs4_len; i++){
        d = mgVec4Len2(&vecs4[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(vec4Len)
{
    mg_real_t d;
    size_t i;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs4_len; i++){
        d = mgVec4Len(&vecs4[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec4Dist2)
{
    mg_real_t d;
    size_t i;

    printf("# ---- dist2 ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        d = mgVec4Dist2(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist2 end ----\n\n");
}

TEST(vec4Dist)
{
    mg_real_t d;
    size_t i;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs4_len - 1; i++){
        d = mgVec4Dist(&vecs4[i], &vecs4[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist end ----\n\n");
}

