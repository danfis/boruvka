#include <cu/cu.h>
#include <mg/quat.h>
#include <mg/dbg.h>

#include "data.h"

TEST(quatCore)
{
    mg_quat_t *a, b;
    mg_vec3_t v1, v2;

    a = mgQuatNew(0., 1., 3., 1.);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 1.));
    assertTrue(mgEq(mgQuatZ(a), 3.));
    assertTrue(mgEq(mgQuatW(a), 1.));

    mgQuatSet(a, 1., 3., 4., 5.);
    assertTrue(mgEq(mgQuatX(a), 1.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetX(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetY(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 4.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetZ(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 2.));
    assertTrue(mgEq(mgQuatW(a), 5.));

    mgQuatSetW(a, 2.);
    assertTrue(mgEq(mgQuatX(a), 2.));
    assertTrue(mgEq(mgQuatY(a), 2.));
    assertTrue(mgEq(mgQuatZ(a), 2.));
    assertTrue(mgEq(mgQuatW(a), 2.));

    assertTrue(mgEq(mgQuatLen2(a), 4. * 4.));

    mgQuatScale(a, 0.5);
    assertTrue(mgEq(mgQuatX(a), 1.));
    assertTrue(mgEq(mgQuatY(a), 1.));
    assertTrue(mgEq(mgQuatZ(a), 1.));
    assertTrue(mgEq(mgQuatW(a), 1.));

    mgQuatNormalize(a);
    assertTrue(mgEq(mgQuatX(a), .5));
    assertTrue(mgEq(mgQuatY(a), .5));
    assertTrue(mgEq(mgQuatZ(a), .5));
    assertTrue(mgEq(mgQuatW(a), .5));

    mgQuatSet(&b, 2., 3., 4., 5.);
    mgQuatMul(a, &b);
    assertTrue(mgEq(mgQuatX(a), 4.));
    assertTrue(mgEq(mgQuatY(a), 3.));
    assertTrue(mgEq(mgQuatZ(a), 5.));
    assertTrue(mgEq(mgQuatW(a), -2.));

    mgQuatSet(a, 1., 2., 1., 2.);
    mgQuatInvert(a);
    assertTrue(mgEq(mgQuatX(a), -.1));
    assertTrue(mgEq(mgQuatY(a), -.2));
    assertTrue(mgEq(mgQuatZ(a), -.1));
    assertTrue(mgEq(mgQuatW(a),  .2));

    mgVec3Set(&v1, 0, 0, 1);
    mgQuatSetAngleAxis(a, M_PI_2, &v1);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 0.));
    assertTrue(mgEq(mgQuatZ(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgQuatW(a), MG_SQRT(2.)/2.));

    mgVec3Set(&v2, 1., 0., 1.);
    mgQuatRotVec(&v2, a);
    assertTrue(mgEq(mgQuatX(a), 0.));
    assertTrue(mgEq(mgQuatY(a), 0.));
    assertTrue(mgEq(mgQuatZ(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgQuatW(a), MG_SQRT(2.)/2.));
    assertTrue(mgEq(mgVec3Y(&v2), 1.));
    assertTrue(mgEq(mgVec3Z(&v2), 1.));

    mgQuatDel(a);
}


TEST(quatLen2)
{
    size_t i;
    mg_real_t l;

    printf("# ---- len2 ----\n");
    for (i = 0; i < quats_len; i++){
        l = mgQuatLen2(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(quatLen)
{
    size_t i;
    mg_real_t l;

    printf("# ---- len ----\n");
    for (i = 0; i < quats_len; i++){
        l = mgQuatLen(&quats[i]);
        printf("# %g\n", l);
    }
    printf("# ---- len end ----\n\n");
}

TEST(quatNormalize)
{
    size_t i;
    mg_quat_t q;

    printf("# ---- normalize ----\n");
    for (i = 0; i < quats_len; i++){
        mgQuatCopy(&q, &quats[i]);
        mgQuatNormalize(&q);
        printf("# %g %g %g %g\n", mgQuatX(&q), mgQuatY(&q), mgQuatZ(&q), mgQuatW(&q));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(quatScale)
{
    size_t i;
    mg_quat_t q;

    printf("# ---- scale ----\n");
    for (i = 0; i < quats_len; i++){
        mgQuatCopy(&q, &quats[i]);
        mgQuatScale(&q, mgQuatX(&quats[i]));
        printf("# %g %g %g %g\n", mgQuatX(&q), mgQuatY(&q), mgQuatZ(&q), mgQuatW(&q));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(quatMul)
{
    size_t i;
    mg_quat_t q, q2;

    printf("# ---- mul ----\n");
    for (i = 0; i < quats_len - 1; i++){
        mgQuatCopy(&q, &quats[i]);
        mgQuatMul(&q, &quats[i + 1]);
        mgQuatMul2(&q2, &quats[i], &quats[i + 1]);
        printf("# %g %g %g %g\n", mgQuatX(&q), mgQuatY(&q), mgQuatZ(&q), mgQuatW(&q));
        printf("# %g %g %g %g\n", mgQuatX(&q2), mgQuatY(&q2), mgQuatZ(&q2), mgQuatW(&q2));
    }
    printf("# ---- mul end ----\n\n");
}


TEST(quatInvert)
{
    size_t i;
    mg_quat_t q, q2;

    printf("# ---- invert ----\n");
    for (i = 0; i < quats_len; i++){
        mgQuatCopy(&q, &quats[i]);
        mgQuatInvert(&q);
        mgQuatInvert2(&q2, &quats[i]);
        printf("# %g %g %g %g\n", mgQuatX(&q), mgQuatY(&q), mgQuatZ(&q), mgQuatW(&q));
        printf("# %g %g %g %g\n", mgQuatX(&q2), mgQuatY(&q2), mgQuatZ(&q2), mgQuatW(&q2));
    }
    printf("# ---- invert end ----\n\n");
}

TEST(quatRotVec)
{
    size_t i, j;
    mg_vec3_t v;

    printf("# ---- rot vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        mgVec3Copy(&v, &vecs[j]);
        mgQuatRotVec(&v, &quats[i]);
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- rot vec end ----\n\n");
}

TEST(quatAngleAxis)
{
    size_t i, j;
    mg_quat_t q;
    mg_vec3_t v;
    mg_real_t a;

    printf("# ---- angle axis vec ----\n");
    for (i = 0, j = 0; i < quats_len && j < vecs_len; i++, j++){
        mgVec3Copy(&v, &vecs[j]);
        a = mgQuatX(&quats[i]) * M_PI_2;
        mgQuatSetAngleAxis(&q, a, &v);
        printf("# %g %g %g %g\n", mgQuatX(&q), mgQuatY(&q), mgQuatZ(&q), mgQuatW(&q));
    }
    printf("# ---- angle axis end ----\n\n");
}
