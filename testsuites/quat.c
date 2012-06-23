#include <cu/cu.h>
#include <boruvka/quat.h>
#include <boruvka/dbg.h>

#include "data.h"

TEST(quatCore)
{
    fer_quat_t *a, b;
    fer_vec3_t v1, v2;

    a = ferQuatNew(0., 1., 3., 1.);
    assertTrue(ferEq(ferQuatX(a), 0.));
    assertTrue(ferEq(ferQuatY(a), 1.));
    assertTrue(ferEq(ferQuatZ(a), 3.));
    assertTrue(ferEq(ferQuatW(a), 1.));

    ferQuatSet(a, 1., 3., 4., 5.);
    assertTrue(ferEq(ferQuatX(a), 1.));
    assertTrue(ferEq(ferQuatY(a), 3.));
    assertTrue(ferEq(ferQuatZ(a), 4.));
    assertTrue(ferEq(ferQuatW(a), 5.));

    ferQuatSetX(a, 2.);
    assertTrue(ferEq(ferQuatX(a), 2.));
    assertTrue(ferEq(ferQuatY(a), 3.));
    assertTrue(ferEq(ferQuatZ(a), 4.));
    assertTrue(ferEq(ferQuatW(a), 5.));

    ferQuatSetY(a, 2.);
    assertTrue(ferEq(ferQuatX(a), 2.));
    assertTrue(ferEq(ferQuatY(a), 2.));
    assertTrue(ferEq(ferQuatZ(a), 4.));
    assertTrue(ferEq(ferQuatW(a), 5.));

    ferQuatSetZ(a, 2.);
    assertTrue(ferEq(ferQuatX(a), 2.));
    assertTrue(ferEq(ferQuatY(a), 2.));
    assertTrue(ferEq(ferQuatZ(a), 2.));
    assertTrue(ferEq(ferQuatW(a), 5.));

    ferQuatSetW(a, 2.);
    assertTrue(ferEq(ferQuatX(a), 2.));
    assertTrue(ferEq(ferQuatY(a), 2.));
    assertTrue(ferEq(ferQuatZ(a), 2.));
    assertTrue(ferEq(ferQuatW(a), 2.));

    assertTrue(ferEq(ferQuatLen2(a), 4. * 4.));

    ferQuatScale(a, 0.5);
    assertTrue(ferEq(ferQuatX(a), 1.));
    assertTrue(ferEq(ferQuatY(a), 1.));
    assertTrue(ferEq(ferQuatZ(a), 1.));
    assertTrue(ferEq(ferQuatW(a), 1.));

    ferQuatNormalize(a);
    assertTrue(ferEq(ferQuatX(a), .5));
    assertTrue(ferEq(ferQuatY(a), .5));
    assertTrue(ferEq(ferQuatZ(a), .5));
    assertTrue(ferEq(ferQuatW(a), .5));

    ferQuatSet(&b, 2., 3., 4., 5.);
    ferQuatMul(a, &b);
    assertTrue(ferEq(ferQuatX(a), 4.));
    assertTrue(ferEq(ferQuatY(a), 3.));
    assertTrue(ferEq(ferQuatZ(a), 5.));
    assertTrue(ferEq(ferQuatW(a), -2.));

    ferQuatSet(a, 1., 2., 1., 2.);
    ferQuatInvert(a);
    assertTrue(ferEq(ferQuatX(a), -.1));
    assertTrue(ferEq(ferQuatY(a), -.2));
    assertTrue(ferEq(ferQuatZ(a), -.1));
    assertTrue(ferEq(ferQuatW(a),  .2));

    ferVec3Set(&v1, 0, 0, 1);
    ferQuatSetAngleAxis(a, M_PI_2, &v1);
    assertTrue(ferEq(ferQuatX(a), 0.));
    assertTrue(ferEq(ferQuatY(a), 0.));
    assertTrue(ferEq(ferQuatZ(a), FER_SQRT(2.)/2.));
    assertTrue(ferEq(ferQuatW(a), FER_SQRT(2.)/2.));

    ferVec3Set(&v2, 1., 0., 1.);
    ferQuatRotVec(&v2, a);
    assertTrue(ferEq(ferQuatX(a), 0.));
    assertTrue(ferEq(ferQuatY(a), 0.));
    assertTrue(ferEq(ferQuatZ(a), FER_SQRT(2.)/2.));
    assertTrue(ferEq(ferQuatW(a), FER_SQRT(2.)/2.));
    assertTrue(ferEq(ferVec3Y(&v2), 1.));
    assertTrue(ferEq(ferVec3Z(&v2), 1.));

    ferQuatDel(a);
}


TEST(quatLen2)
{
    size_t i;
    fer_real_t l;

    printf("# ---- len2 ----\n");
    for (i = 0; i < quats_len; i++){
        l = ferQuatLen2(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(quatLen)
{
    size_t i;
    fer_real_t l;

    printf("# ---- len ----\n");
    for (i = 0; i < quats_len; i++){
        l = ferQuatLen(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len end ----\n\n");
}

TEST(quatNormalize)
{
    size_t i;
    fer_quat_t q;

    printf("# ---- normalize ----\n");
    for (i = 0; i < quats_len; i++){
        ferQuatCopy(&q, &quats[i]);
        ferQuatNormalize(&q);
        printf("# %g %g %g %g\n", ferQuatX(&q), ferQuatY(&q), ferQuatZ(&q), ferQuatW(&q));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(quatScale)
{
    size_t i;
    fer_quat_t q;

    printf("# ---- scale ----\n");
    for (i = 0; i < quats_len; i++){
        ferQuatCopy(&q, &quats[i]);
        ferQuatScale(&q, ferQuatX(&quats[i]));
        printf("# %g %g %g %g\n", ferQuatX(&q), ferQuatY(&q), ferQuatZ(&q), ferQuatW(&q));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(quatMul)
{
    size_t i;
    fer_quat_t q, q2;

    printf("# ---- mul ----\n");
    for (i = 0; i < quats_len - 1; i++){
        ferQuatCopy(&q, &quats[i]);
        ferQuatMul(&q, &quats[i + 1]);
        ferQuatMul2(&q2, &quats[i], &quats[i + 1]);
        printf("# %g %g %g %g\n", ferQuatX(&q), ferQuatY(&q), ferQuatZ(&q), ferQuatW(&q));
        printf("# %g %g %g %g\n", ferQuatX(&q2), ferQuatY(&q2), ferQuatZ(&q2), ferQuatW(&q2));
    }
    printf("# ---- mul end ----\n\n");
}


TEST(quatInvert)
{
    size_t i;
    fer_quat_t q, q2;

    printf("# ---- invert ----\n");
    for (i = 0; i < quats_len; i++){
        ferQuatCopy(&q, &quats[i]);
        ferQuatInvert(&q);
        ferQuatInvert2(&q2, &quats[i]);
        printf("# %g %g %g %g\n", ferQuatX(&q), ferQuatY(&q), ferQuatZ(&q), ferQuatW(&q));
        printf("# %g %g %g %g\n", ferQuatX(&q2), ferQuatY(&q2), ferQuatZ(&q2), ferQuatW(&q2));
    }
    printf("# ---- invert end ----\n\n");
}

TEST(quatRotVec)
{
    size_t i, j;
    fer_vec3_t v;

    printf("# ---- rot vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        ferVec3Copy(&v, &vecs[j]);
        ferQuatRotVec(&v, &quats[i]);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- rot vec end ----\n\n");
}

TEST(quatAngleAxis)
{
    size_t i, j;
    fer_quat_t q;
    fer_vec3_t v;
    fer_real_t a;

    printf("# ---- angle axis vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        ferVec3Copy(&v, &vecs[j]);
        a = ferQuatX(&quats[i]) * M_PI_2;
        ferQuatSetAngleAxis(&q, a, &v);
        printf("# %g %g %g %g\n", ferQuatX(&q), ferQuatY(&q), ferQuatZ(&q), ferQuatW(&q));
    }
    printf("# ---- angle axis end ----\n\n");
}
