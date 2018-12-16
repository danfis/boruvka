#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/vec3.h>
#include <boruvka/dbg.h>
#include "data.h"

static void projToPlanePrint(bor_vec3_t *vs, int num);

TEST(vec3SetUp)
{
}

TEST(vec3TearDown)
{
}

TEST(vec3Alloc)
{
    bor_vec3_t *v, w;

    v = borVec3New(0., 1., 2.);
    borVec3Set(&w, 0., 1., 2.);
    assertTrue(borVec3Eq(v, &w));
    borVec3Del(v);
}

TEST(vec3Add)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- add ----\n");
    borVec3Set(&v, BOR_ZERO, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs_len; i++){
        borVec3Add(&v, &vecs[i]);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec3Sub)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- sub ----\n");
    borVec3Set(&v, BOR_ZERO, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs_len; i++){
        borVec3Sub(&v, &vecs[i]);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec3Scale)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- scale ----\n");
    borVec3Copy(&v, &vecs[0]);
    for (i = 0; i < vecs_len; i++){
        borVec3Scale(&v, borVec3X(&vecs[i]));
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec3Normalize)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs_len; i++){
        borVec3Copy(&v, &vecs[i]);
        borVec3Normalize(&v);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec3Dot)
{
    bor_real_t dot;
    size_t i;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        dot = borVec3Dot(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec3Mul)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- mul ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        borVec3MulComp2(&v, &vecs[i], &vecs[i + 1]);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- mul end ----\n\n");
}

TEST(vec3Cross)
{
    bor_vec3_t v;
    size_t i;

    printf("# ---- cross ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        borVec3Cross(&v, &vecs[i], &vecs[i + 1]);
        printf("# %g %g %g\n", borVec3X(&v), borVec3Y(&v), borVec3Z(&v));
    }
    printf("# ---- cross end ----\n\n");
}

TEST(vec3Len2)
{
    bor_real_t d;
    size_t i;

    printf("# ---- len2 ----\n");
    for (i = 0; i < vecs_len; i++){
        d = borVec3Len2(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(vec3Len)
{
    bor_real_t d;
    size_t i;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs_len; i++){
        d = borVec3Len(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec3Dist2)
{
    bor_real_t d;
    size_t i;

    printf("# ---- dist2 ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = borVec3Dist2(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist2 end ----\n\n");
}

TEST(vec3Dist)
{
    bor_real_t d;
    size_t i;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = borVec3Dist(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist end ----\n\n");
}

TEST(vec3SegmentDist)
{
    bor_real_t d;
    bor_vec3_t w;
    size_t i;

    printf("# ---- segment dist ----\n");
    for (i = 0; i < vecs_len - 2; i++){
        d = borVec3PointSegmentDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &w);
        printf("# %g %g %g %g\n", d, borVec3X(&w), borVec3Y(&w), borVec3Z(&w));
    }
    printf("# ---- segment dist end ----\n\n");
}

TEST(vec3TriDist)
{
    bor_real_t d;
    bor_vec3_t w;
    size_t i;

    printf("# ---- tri dist ----\n");
    for (i = 0; i < vecs_len - 3; i++){
        d = borVec3PointTriDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &vecs[i + 3], &w);
        printf("# %g %g %g %g\n", d, borVec3X(&w), borVec3Y(&w), borVec3Z(&w));
    }
    printf("# ---- tri dist end ----\n\n");
}


TEST(vec3Core)
{
    bor_vec3_t a, b, c, d;

    borVec3Set(&a, 0., 0., 0.);
    assertTrue(borVec3Eq(&a, bor_vec3_origin));

    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(borVec3Eq(&a, &b));

    borVec3Set(&a, 9., 1., 5.);
    borVec3Copy(&b, &a);
    assertTrue(borVec3Eq(&a, &b));

    borVec3Set(&a, 0., 0., 0.);
    assertTrue(borEq(0., borVec3Len2(&a)));
    borVec3Set(&a, 1., 1., 1.);
    assertTrue(borEq(3., borVec3Len2(&a)));
    borVec3Set(&a, 1., 4., 3.);
    assertTrue(borEq(1. + 4. * 4. + 3. * 3., borVec3Len2(&a)));

    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(borEq(0., borVec3Dist2(&a, &b)));
    borVec3Set(&a, 0., 1., .4);
    borVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(borEq(10.04, borVec3Dist2(&a, &b)));

    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Set(&b, 1., 1.2, 3.4);
    borVec3Sub2(&c, &a, &b);
    assertTrue(borVec3Eq(&c, bor_vec3_origin));
    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Set(&b, 1., 1.2, 3.4);
    borVec3Sub(&a, &b);
    assertTrue(borVec3Eq(&a, bor_vec3_origin));
    borVec3Set(&a, 1.3, 1.2, 3.4);
    borVec3Set(&b, 0.2,  .2,  .4);
    borVec3Sub2(&c, &a, &b);
    borVec3Set(&d, 1.1, 1., 3.);
    assertTrue(borVec3Eq(&c, &d));
    borVec3Sub(&b, &a);
    borVec3Set(&d, -1.1, -1., -3.);
    assertTrue(borVec3Eq(&b, &d));

    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Set(&b, 1., 1.2, 3.4);
    borVec3Add(&a, &b);
    borVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(borVec3Eq(&a, &d));
    borVec3Set(&a, 1.3, 1.2, 3.4);
    borVec3Set(&b, 0.2,  .2,  .4);
    borVec3Add(&a, &b);
    borVec3Set(&d, 1.5, 1.4, 3.8);
    assertTrue(borVec3Eq(&a, &d));

    borVec3Set(&a, 1., 1.2, 3.4);
    borVec3Scale(&a, 2.);
    borVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(borVec3Eq(&a, &d));
    borVec3Set(&a, 1.3, 1.2, 3.4);
    borVec3Scale(&a, 0.2);
    borVec3Set(&d, 1.3 * .2, 1.2 * .2, 3.4 * .2);
    assertTrue(borVec3Eq(&a, &d));

    borVec3Set(&a, 2., 1.1, 5.4);
    borVec3Normalize(&a);
    assertTrue(borEq(1., borVec3Len2(&a)));
    borVec3Set(&a, 1., .1, 3.4);
    borVec3Normalize(&a);
    assertTrue(borEq(1., borVec3Len2(&a)));

    borVec3Set(&a, 2., 1.1, 5.4);
    assertTrue(borEq(borVec3Len2(&a), borVec3Dot(&a, &a)));
    borVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(borEq(2. + 1.1 * 1.2 + 5.4 * 3.4, borVec3Dot(&a, &b)));
    borVec3Set(&a, 2., 2.4, 6.8);
    assertTrue(borEq(2. + 2.4 * 1.2 + 6.8 * 3.4, borVec3Dot(&a, &b)));

    borVec3Set(&a, 1., 0., 0.);
    borVec3Set(&b, 0., 1., 0.);
    borVec3Cross(&c, &a, &b);
    borVec3Set(&d, 0., 0., 1.);
    assertTrue(borVec3Eq(&c, &d));
    borVec3Cross(&c, &b, &a);
    borVec3Set(&d, 0., 0., -1.);
    assertTrue(borVec3Eq(&c, &d));
    borVec3Set(&a, 1., 1., 1.);
    borVec3Set(&b, 0., 1., 0.);
    borVec3Cross(&c, &a, &b);
    borVec3Set(&d, -1., 0., 1.);
    assertTrue(borVec3Eq(&c, &d));
    borVec3Cross(&c, &b, &a);
    borVec3Set(&d, 1., 0., -1.);
    assertTrue(borVec3Eq(&c, &d));
}

TEST(vec3PointSegmentDist)
{
    bor_vec3_t P, a, b, w, ew;
    bor_real_t dist;

    borVec3Set(&a, 0., 0., 0.);
    borVec3Set(&b, 1., 0., 0.);

    // extereme w == a
    borVec3Set(&P, -1., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 1.));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -0.5, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.5 * 0.5));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -0.1, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, .1 * .1));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, 0., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -1., 1., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 2.));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -0.5, 0.5, 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.5));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -0.1, -1., 2.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 5.01));
    assertTrue(borVec3Eq(&w, &a));


    // extereme w == b
    borVec3Set(&P, 2., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 1.));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 1.5, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.5 * 0.5));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 1.1, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, .1 * .1));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 1., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 2., 1., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 2.));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 1.5, 0.5, 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.5));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 1.1, -1., 2.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 5.01));
    assertTrue(borVec3Eq(&w, &b));

    // inside segment
    borVec3Set(&P, .5, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &P));

    borVec3Set(&P, .9, 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &P));

    borVec3Set(&P, .5, 1., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 1.));
    borVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(borVec3Eq(&w, &ew));

    borVec3Set(&P, .5, 1., 1.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 2.));
    borVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(borVec3Eq(&w, &ew));



    borVec3Set(&a, -.5, 2., 1.);
    borVec3Set(&b, 1., 1.5, 0.5);

    // extereme w == a
    borVec3Set(&P, -10., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 9.5 * 9.5 + 2. * 2. + 1.));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, -10., 9.2, 3.4);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 9.5 * 9.5 + 7.2 * 7.2 + 2.4 * 2.4));
    assertTrue(borVec3Eq(&w, &a));

    // extereme w == b
    borVec3Set(&P, 10., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 9. * 9. + 1.5 * 1.5 + 0.5 * 0.5));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, 10., 9.2, 3.4);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 9. * 9. + 7.7 * 7.7 + 2.9 * 2.9));
    assertTrue(borVec3Eq(&w, &b));

    // inside ab
    borVec3Set(&a, -.1, 1., 1.);
    borVec3Set(&b, 1., 1., 1.);
    borVec3Set(&P, 0., 0., 0.);
    dist = borVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(borEq(dist, 2.));
    borVec3Set(&ew, 0., 1., 1.);
    assertTrue(borVec3Eq(&w, &ew));
}


TEST(vec3PointTriDist)
{
    bor_vec3_t P, a, b, c, w, P0;
    bor_real_t dist;

    borVec3Set(&a, -1., 0., 0.);
    borVec3Set(&b, 0., 1., 1.);
    borVec3Set(&c, -1., 0., 1.);

    borVec3Set(&P, -1., 0., 0.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &a));

    borVec3Set(&P, 0., 1., 1.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &b));

    borVec3Set(&P, -1., 0., 1.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &c));

    borVec3Set(&P, 0., 0., 0.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, NULL);
    assertTrue(borEq(dist, 2./3.));


    // region 4
    borVec3Set(&P, -2., 0., 0.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &a)));
    assertTrue(borVec3Eq(&w, &a));
    borVec3Set(&P, -2., 0.2, -1.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &a)));
    assertTrue(borVec3Eq(&w, &a));

    // region 2
    borVec3Set(&P, -1.3, 0., 1.2);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &c)));
    assertTrue(borVec3Eq(&w, &c));
    borVec3Set(&P, -1.2, 0.2, 1.1);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &c)));
    assertTrue(borVec3Eq(&w, &c));

    // region 6
    borVec3Set(&P, 0.3, 1., 1.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &b)));
    assertTrue(borVec3Eq(&w, &b));
    borVec3Set(&P, .1, 1., 1.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, borVec3Dist2(&P, &b)));
    assertTrue(borVec3Eq(&w, &b));

    // region 1
    borVec3Set(&P, 0., 1., 2.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 1.));
    assertTrue(borVec3Eq(&w, &b));
    borVec3Set(&P, -1., 0., 2.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 1.));
    assertTrue(borVec3Eq(&w, &c));
    borVec3Set(&P, -0.5, 0.5, 2.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 1.));
    borVec3Set(&P0, -0.5, 0.5, 1.);
    assertTrue(borVec3Eq(&w, &P0));

    // region 3
    borVec3Set(&P, -2., -1., 0.7);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 2.));
    borVec3Set(&P0, -1., 0., 0.7);
    assertTrue(borVec3Eq(&w, &P0));

    // region 5
    borVec3Set(&P, 0., 0., 0.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 2./3.));
    borVec3Set(&P0, -2./3., 1./3., 1./3.);
    assertTrue(borVec3Eq(&w, &P0));

    // region 0
    borVec3Set(&P, -0.5, 0.5, 0.5);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &P));
    borVec3Set(&P, -0.5, 0.5, 0.7);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &P));
    borVec3Set(&P, -0.5, 0.5, 0.9);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.));
    assertTrue(borVec3Eq(&w, &P));

    borVec3Set(&P, 0., 0., 0.5);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.5));
    borVec3Set(&P0, -.5, .5, .5);
    assertTrue(borVec3Eq(&w, &P0));

    borVec3Set(&a, -1., 0., 0.);
    borVec3Set(&b, 0., 1., -1.);
    borVec3Set(&c, 0., 1., 1.);
    borVec3Set(&P, 0., 0., 0.);
    dist = borVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(borEq(dist, 0.5));
    borVec3Set(&P0, -.5, .5, 0.);
    assertTrue(borVec3Eq(&w, &P0));
    //fprintf(stderr, "dist: %lf\n", dist);
}

TEST(vec3PointInTri)
{
    bor_vec3_t v[5];

    borVec3Set(&v[0], 1., 1., 0.);
    borVec3Set(&v[1], 0., 0., 0.);
    borVec3Set(&v[2], 2., 0.5, 0.);
    borVec3Set(&v[3], 0., 2., 0.);
    assertTrue(borVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    borVec3Set(&v[0], 1., 1., 0.);
    borVec3Set(&v[1], 0., 0., 0.);
    borVec3Set(&v[2], 2., -1., 0.);
    borVec3Set(&v[3], 0., 2., 0.);
    assertFalse(borVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    borVec3Set(&v[0], 1., 1., 0.);
    borVec3Set(&v[1], 0., 0., 0.);
    borVec3Set(&v[2], 0., 0., 0.);
    borVec3Set(&v[3], 0., 0., 0.);
    assertFalse(borVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    borVec3Set(&v[0], 1., 1., 0.);
    borVec3Set(&v[1], 1., 0., 0.);
    borVec3Set(&v[2], 0., 0., 0.);
    borVec3Set(&v[3], -1., 0., 0.);
    assertFalse(borVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    borVec3Set(&v[4], 0., 0., 0.);
    borVec3Set(&v[1], -1., 1., -1.);
    borVec3Set(&v[2], 1., 1., -1.);
    borVec3Set(&v[3], 0., 1., 2.);
    borVec3ProjToPlane(&v[4], &v[1], &v[2], &v[3], &v[0]);
    assertTrue(borVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    borVec3Set(&v[1], 0., 0., 0.);
    borVec3Set(&v[2], -1., 1., -1.);
    borVec3Set(&v[3], 1., 0.8, -1.);
    borVec3Set(&v[4], 0., 1.3, 2.2);
    borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]);
    assertTrue(borVec3PointInTri(&v[0], &v[2], &v[3], &v[4]));
}

TEST(vec3Angle)
{
    bor_vec3_t v[3];
    bor_real_t a;
    size_t i;

    borVec3Set(&v[0], 2., 2., 0.);
    borVec3Set(&v[1], 1., 0., 0.);
    a = borVec3Angle(&v[0], bor_vec3_origin, &v[1]);
    assertTrue(borEq(a, M_PI_4));

    borVec3Set(&v[0], 1., 2., 1.);
    borVec3Set(&v[1], 0., 1., 0.);
    borVec3Set(&v[2], -1., 0., -1.);
    a = borVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(borEq(a, M_PI));

    borVec3Set(&v[0], 0., 1., 1.);
    borVec3Set(&v[1], 0., 1., 0.);
    borVec3Set(&v[2], -1., 1., 0.);
    a = borVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(borEq(a, M_PI_2));

    borVec3Set(&v[0], 0., 1., 1.);
    borVec3Set(&v[1], 0., 1., 0.);
    borVec3Set(&v[2], -10., 11., 0.);
    a = borVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(borEq(a, M_PI_2));

    borVec3Set(&v[0], 0., 1., 1.);
    borVec3Set(&v[1], 0., 1., 0.);
    borVec3Set(&v[2], 0.01, 0.06, 0.);
    a = borVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(borEq(a, M_PI_2));

    printf("# ---- angle ----\n");
    for (i = 0; i < vecs_len - 2; i++){
        a = borVec3Angle(&vecs[i], &vecs[i + 1], &vecs[i + 2]);
        printf("# %g\n", a);
    }
    printf("# ---- angle end ----\n\n");
}

static void projToPlanePrint(bor_vec3_t *vs, int num)
{
    size_t i;

    printf("------\n");
    printf("Point color: 0.8 0.8 0.8\n");
    printf("Face color: 0 1 0\n");
    printf("Name: Proj %d - Plane\n", num);
    printf("Points:\n");
    for (i=0; i < 3; i++)
        printf("%g %g %g\n", borVec3X(&vs[i + 2]), borVec3Y(&vs[i + 2]), borVec3Z(&vs[i + 2]));
    printf("Faces: 0 1 2\n");

    printf("------\n");
    printf("Point color: 1 0 0 \n");
    printf("Name: Proj %d - OPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", borVec3X(&vs[1]), borVec3Y(&vs[1]), borVec3Z(&vs[1]));

    printf("------\n");
    printf("Point color: 0 0 1 \n");
    printf("Name: Proj %d - PPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", borVec3X(&vs[0]), borVec3Y(&vs[0]), borVec3Z(&vs[0]));
}

TEST(vec3ProjToPlane)
{
    bor_vec3_t v[6];

    borVec3Set(&v[1], 0., 0., 0.);
    borVec3Set(&v[2], 0., 0., 0.);
    borVec3Set(&v[3], 1., 0., 0.);
    borVec3Set(&v[4], 0., 1., 0.);
    borVec3Set(&v[5], 0., 0., 0.);
    assertTrue(borEq(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]), BOR_ZERO));
    assertTrue(borVec3Eq(&v[0], &v[5]));

    borVec3Set(&v[1], 0., 0., 1.);
    borVec3Set(&v[2], 0., 0., 0.);
    borVec3Set(&v[3], 1., 0., 0.);
    borVec3Set(&v[4], 0., 1., 0.);
    borVec3Set(&v[5], 0., 0., 0.);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    assertTrue(borVec3Eq(&v[0], &v[5]));

    borVec3Set(&v[1], 0., 0., -1.);
    borVec3Set(&v[2], 0., 0., 0.);
    borVec3Set(&v[3], 1., 0., 0.);
    borVec3Set(&v[4], 0., 1., 0.);
    borVec3Set(&v[5], 0., 0., 0.);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    assertTrue(borVec3Eq(&v[0], &v[5]));

    borVec3Set(&v[1], 0., 0., 1.);
    borVec3Set(&v[2], -1., -1., -1.);
    borVec3Set(&v[3], 1., -1., -1.);
    borVec3Set(&v[4], -1., 1., -1.);
    borVec3Set(&v[5], 0., 0., -1.);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    assertTrue(borVec3Eq(&v[0], &v[5]));

    borVec3Set(&v[1], 0.5, 10., 0.5);
    borVec3Set(&v[2], 1., 0., 0.);
    borVec3Set(&v[3], -1., 0., 1.);
    borVec3Set(&v[4], -1., 0., -1.);
    borVec3Set(&v[5], 0.5, 0., 0.5);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    assertTrue(borVec3Eq(&v[0], &v[5]));

    borVec3Set(&v[1], 0.5, 10., 0.5);
    borVec3Set(&v[2], -1., -1., -1.);
    borVec3Set(&v[3], 1., 1., 1.);
    borVec3Set(&v[4], 0., 0., 0.);
    assertFalse(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);

    borVec3Set(&v[1], 0., 0., 1.);
    borVec3Set(&v[2], -1., -1., -1.);
    borVec3Set(&v[3], 1., 1., 1.);
    borVec3Set(&v[4], -1., 1., 1.);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    projToPlanePrint(v, 1);

    borVec3Set(&v[1], 3., -2., 1.);
    borVec3Set(&v[2], -1., -2., -1.);
    borVec3Set(&v[3], 1., 2., 1.8);
    borVec3Set(&v[4], -1.4, 1., 1.2);
    assertTrue(borVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > BOR_ZERO);
    projToPlanePrint(v, 2);
}


TEST(vec3Centroid)
{
    bor_vec3_t v[5];

    borVec3Set(&v[0], 0., 0., 0.);
    borVec3Set(&v[1], 1., 0., 0.);
    borVec3Set(&v[2], 0., 1., 0.);
    borVec3TriCentroid(&v[0], &v[1], &v[2], &v[3]);

    borVec3Set(&v[4], 1./3., 1./3., 0.);
    assertTrue(borVec3Eq(&v[3], &v[4]));
}


TEST(vec3TriTriOverlap)
{
    bor_vec3_t p1, q1, r1, p2, q2, r2;
    int res;
    size_t i;

    borVec3Set(&p1, 0, 0, 0);
    borVec3Set(&q1, 1, 0, 0);
    borVec3Set(&r1, 1, 1, 0);
    borVec3Set(&p2, 0, 0, 1);
    borVec3Set(&q2, 1, 0, 1);
    borVec3Set(&r2, 1, 1, 1);
    res = borVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertFalse(res);

    borVec3Set(&p1, 0, 0, 0);
    borVec3Set(&q1, 1, 0, 0);
    borVec3Set(&r1, 1, 1, 0);
    borVec3Set(&p2, 0.1, 0, 0);
    borVec3Set(&q2, 1.1, 0, 0);
    borVec3Set(&r2, 1.1, 1, 0);
    res = borVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertTrue(res);

    borVec3Set(&p1, 0, 0, 0);
    borVec3Set(&q1, 1, 0, 0);
    borVec3Set(&r1, 1, 1, 0);
    borVec3Set(&p2, 0.5, 0.5, 0.5);
    borVec3Set(&q2, 0.5, 0.5, -1);
    borVec3Set(&r2, -1, 0, 0);
    res = borVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertTrue(res);

    printf("# ---- tri tri overlap ----\n");
    for (i = 0; i < vecs_len / 100; i += 6){
        res = borVec3TriTriOverlap(&vecs[i + 0], &vecs[i + 1], &vecs[i + 2],
                                   &vecs[i + 3], &vecs[i + 4], &vecs[i + 5]);
        printf("# res[%04d]: %d\n", (int)i, (int)res);
        /*
        char name[120];
        sprintf(name, "v: %d", res);
        printf("--\nName: [%04u] %s\n", i, name);
        printf("Points:\n");
        borVec3Print(&vecs[i + 0], stdout); printf("\n");
        borVec3Print(&vecs[i + 1], stdout); printf("\n");
        borVec3Print(&vecs[i + 2], stdout); printf("\n");
        printf("Edges:\n0 1\n1 2\n2 0\n--\n");
        printf("---\nPoints:\n");
        borVec3Print(&vecs[i + 3], stdout); printf("\n");
        borVec3Print(&vecs[i + 4], stdout); printf("\n");
        borVec3Print(&vecs[i + 5], stdout); printf("\n");
        printf("Edges:\n0 1\n1 2\n2 0\n--\n");
        */
    }
    printf("# ---- tri tri overlap end ----\n\n");
}
