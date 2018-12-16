#include <cu/cu.h>
#include <boruvka/quat.h>
#include <boruvka/dbg.h>

#include "data.h"

TEST(quatCore)
{
    bor_quat_t *a, b;
    bor_vec3_t v1, v2;

    a = borQuatNew(0., 1., 3., 1.);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 1.));
    assertTrue(borEq(borQuatZ(a), 3.));
    assertTrue(borEq(borQuatW(a), 1.));

    borQuatSet(a, 1., 3., 4., 5.);
    assertTrue(borEq(borQuatX(a), 1.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetX(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetY(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 4.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetZ(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 2.));
    assertTrue(borEq(borQuatW(a), 5.));

    borQuatSetW(a, 2.);
    assertTrue(borEq(borQuatX(a), 2.));
    assertTrue(borEq(borQuatY(a), 2.));
    assertTrue(borEq(borQuatZ(a), 2.));
    assertTrue(borEq(borQuatW(a), 2.));

    assertTrue(borEq(borQuatLen2(a), 4. * 4.));

    borQuatScale(a, 0.5);
    assertTrue(borEq(borQuatX(a), 1.));
    assertTrue(borEq(borQuatY(a), 1.));
    assertTrue(borEq(borQuatZ(a), 1.));
    assertTrue(borEq(borQuatW(a), 1.));

    borQuatNormalize(a);
    assertTrue(borEq(borQuatX(a), .5));
    assertTrue(borEq(borQuatY(a), .5));
    assertTrue(borEq(borQuatZ(a), .5));
    assertTrue(borEq(borQuatW(a), .5));

    borQuatSet(&b, 2., 3., 4., 5.);
    borQuatMul(a, &b);
    assertTrue(borEq(borQuatX(a), 4.));
    assertTrue(borEq(borQuatY(a), 3.));
    assertTrue(borEq(borQuatZ(a), 5.));
    assertTrue(borEq(borQuatW(a), -2.));

    borQuatSet(a, 1., 2., 1., 2.);
    borQuatInvert(a);
    assertTrue(borEq(borQuatX(a), -.1));
    assertTrue(borEq(borQuatY(a), -.2));
    assertTrue(borEq(borQuatZ(a), -.1));
    assertTrue(borEq(borQuatW(a),  .2));

    borVec3Set(&v1, 0, 0, 1);
    borQuatSetAngleAxis(a, M_PI_2, &v1);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 0.));
    assertTrue(borEq(borQuatZ(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borQuatW(a), BOR_SQRT(2.)/2.));

    borVec3Set(&v2, 1., 0., 1.);
    borQuatRotVec(&v2, a);
    assertTrue(borEq(borQuatX(a), 0.));
    assertTrue(borEq(borQuatY(a), 0.));
    assertTrue(borEq(borQuatZ(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borQuatW(a), BOR_SQRT(2.)/2.));
    assertTrue(borEq(borVec3Y(&v2), 1.));
    assertTrue(borEq(borVec3Z(&v2), 1.));

    borQuatDel(a);
}


TEST(quatLen2)
{
    size_t i;
    bor_real_t l;

    printf("# ---- len2 ----\n");
    for (i = 0; i < quats_len; i++){
        l = borQuatLen2(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(quatLen)
{
    size_t i;
    bor_real_t l;

    printf("# ---- len ----\n");
    for (i = 0; i < quats_len; i++){
        l = borQuatLen(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len end ----\n\n");
}

TEST(quatNormalize)
{
    size_t i;
    bor_quat_t q;

    printf("# ---- normalize ----\n");
    for (i = 0; i < quats_len; i++){
        borQuatCopy(&q, &quats[i]);
        borQuatNormalize(&q);
        printf("# %g %g %g %g\n", borQuatX(&q), borQuatY(&q), borQuatZ(&q), borQuatW(&q));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(quatScale)
{
    size_t i;
    bor_quat_t q;

    printf("# ---- scale ----\n");
    for (i = 0; i < quats_len; i++){
        borQuatCopy(&q, &quats[i]);
        borQuatScale(&q, borQuatX(&quats[i]));
        printf("# %g %g %g %g\n", borQuatX(&q), borQuatY(&q), borQuatZ(&q), borQuatW(&q));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(quatMul)
{
    size_t i;
    bor_quat_t q, q2;

    printf("# ---- mul ----\n");
    for (i = 0; i < quats_len - 1; i++){
        borQuatCopy(&q, &quats[i]);
        borQuatMul(&q, &quats[i + 1]);
        borQuatMul2(&q2, &quats[i], &quats[i + 1]);
        printf("# %g %g %g %g\n", borQuatX(&q), borQuatY(&q), borQuatZ(&q), borQuatW(&q));
        printf("# %g %g %g %g\n", borQuatX(&q2), borQuatY(&q2), borQuatZ(&q2), borQuatW(&q2));
    }
    printf("# ---- mul end ----\n\n");
}


TEST(quatInvert)
{
    size_t i;
    bor_quat_t q, q2;

    printf("# ---- invert ----\n");
    for (i = 0; i < quats_len; i++){
        borQuatCopy(&q, &quats[i]);
        borQuatInvert(&q);
        borQuatInvert2(&q2, &quats[i]);
        printf("# %g %g %g %g\n", borQuatX(&q), borQuatY(&q), borQuatZ(&q), borQuatW(&q));
        printf("# %g %g %g %g\n", borQuatX(&q2), borQuatY(&q2), borQuatZ(&q2), borQuatW(&q2));
    }
    printf("# ---- invert end ----\n\n");
}

TEST(quatRotVec)
{
    size_t i, j;
    bor_vec3_t v;

    printf("# ---- rot vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        borVec3Copy(&v, &vecs[j]);
        borQuatRotVec(&v, &quats[i]);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- rot vec end ----\n\n");
}

TEST(quatAngleAxis)
{
    size_t i, j;
    bor_quat_t q;
    bor_vec3_t v;
    bor_real_t a;

    printf("# ---- angle axis vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        borVec3Copy(&v, &vecs[j]);
        a = borQuatX(&quats[i]) * M_PI_2;
        borQuatSetAngleAxis(&q, a, &v);
        printf("# %g %g %g %g\n", borQuatX(&q), borQuatY(&q), borQuatZ(&q), borQuatW(&q));
    }
    printf("# ---- angle axis end ----\n\n");
}
