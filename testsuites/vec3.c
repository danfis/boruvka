#include <stdio.h>
#include <cu/cu.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>
#include "data.h"

static void projToPlanePrint(fer_vec3_t *vs, int num);

TEST(vec3SetUp)
{
}

TEST(vec3TearDown)
{
}

TEST(vec3Alloc)
{
    fer_vec3_t *v, w;

    v = ferVec3New(0., 1., 2.);
    ferVec3Set(&w, 0., 1., 2.);
    assertTrue(ferVec3Eq(v, &w));
    ferVec3Del(v);
}

TEST(vec3Add)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- add ----\n");
    ferVec3Set(&v, FER_ZERO, FER_ZERO, FER_ZERO);
    for (i = 0; i < vecs_len; i++){
        ferVec3Add(&v, &vecs[i]);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec3Sub)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- sub ----\n");
    ferVec3Set(&v, FER_ZERO, FER_ZERO, FER_ZERO);
    for (i = 0; i < vecs_len; i++){
        ferVec3Sub(&v, &vecs[i]);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec3Scale)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- scale ----\n");
    ferVec3Copy(&v, &vecs[0]);
    for (i = 0; i < vecs_len; i++){
        ferVec3Scale(&v, ferVec3X(&vecs[i]));
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec3Normalize)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs_len; i++){
        ferVec3Copy(&v, &vecs[i]);
        ferVec3Normalize(&v);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec3Dot)
{
    fer_real_t dot;
    size_t i;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        dot = ferVec3Dot(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec3Mul)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- mul ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        ferVec3MulComp2(&v, &vecs[i], &vecs[i + 1]);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- mul end ----\n\n");
}

TEST(vec3Cross)
{
    fer_vec3_t v;
    size_t i;

    printf("# ---- cross ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        ferVec3Cross(&v, &vecs[i], &vecs[i + 1]);
        printf("# %g %g %g\n", ferVec3X(&v), ferVec3Y(&v), ferVec3Z(&v));
    }
    printf("# ---- cross end ----\n\n");
}

TEST(vec3Len2)
{
    fer_real_t d;
    size_t i;

    printf("# ---- len2 ----\n");
    for (i = 0; i < vecs_len; i++){
        d = ferVec3Len2(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(vec3Len)
{
    fer_real_t d;
    size_t i;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs_len; i++){
        d = ferVec3Len(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec3Dist2)
{
    fer_real_t d;
    size_t i;

    printf("# ---- dist2 ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = ferVec3Dist2(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist2 end ----\n\n");
}

TEST(vec3Dist)
{
    fer_real_t d;
    size_t i;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = ferVec3Dist(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist end ----\n\n");
}

TEST(vec3SegmentDist)
{
    fer_real_t d;
    fer_vec3_t w;
    size_t i;

    printf("# ---- segment dist ----\n");
    for (i = 0; i < vecs_len - 2; i++){
        d = ferVec3PointSegmentDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &w);
        printf("# %g %g %g %g\n", d, ferVec3X(&w), ferVec3Y(&w), ferVec3Z(&w));
    }
    printf("# ---- segment dist end ----\n\n");
}

TEST(vec3TriDist)
{
    fer_real_t d;
    fer_vec3_t w;
    size_t i;

    printf("# ---- tri dist ----\n");
    for (i = 0; i < vecs_len - 3; i++){
        d = ferVec3PointTriDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &vecs[i + 3], &w);
        printf("# %g %g %g %g\n", d, ferVec3X(&w), ferVec3Y(&w), ferVec3Z(&w));
    }
    printf("# ---- tri dist end ----\n\n");
}


TEST(vec3Core)
{
    fer_vec3_t a, b, c, d;

    ferVec3Set(&a, 0., 0., 0.);
    assertTrue(ferVec3Eq(&a, fer_vec3_origin));

    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(ferVec3Eq(&a, &b));

    ferVec3Set(&a, 9., 1., 5.);
    ferVec3Copy(&b, &a);
    assertTrue(ferVec3Eq(&a, &b));

    ferVec3Set(&a, 0., 0., 0.);
    assertTrue(ferEq(0., ferVec3Len2(&a)));
    ferVec3Set(&a, 1., 1., 1.);
    assertTrue(ferEq(3., ferVec3Len2(&a)));
    ferVec3Set(&a, 1., 4., 3.);
    assertTrue(ferEq(1. + 4. * 4. + 3. * 3., ferVec3Len2(&a)));

    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(ferEq(0., ferVec3Dist2(&a, &b)));
    ferVec3Set(&a, 0., 1., .4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(ferEq(10.04, ferVec3Dist2(&a, &b)));

    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    ferVec3Sub2(&c, &a, &b);
    assertTrue(ferVec3Eq(&c, fer_vec3_origin));
    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    ferVec3Sub(&a, &b);
    assertTrue(ferVec3Eq(&a, fer_vec3_origin));
    ferVec3Set(&a, 1.3, 1.2, 3.4);
    ferVec3Set(&b, 0.2,  .2,  .4);
    ferVec3Sub2(&c, &a, &b);
    ferVec3Set(&d, 1.1, 1., 3.);
    assertTrue(ferVec3Eq(&c, &d));
    ferVec3Sub(&b, &a);
    ferVec3Set(&d, -1.1, -1., -3.);
    assertTrue(ferVec3Eq(&b, &d));

    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Set(&b, 1., 1.2, 3.4);
    ferVec3Add(&a, &b);
    ferVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(ferVec3Eq(&a, &d));
    ferVec3Set(&a, 1.3, 1.2, 3.4);
    ferVec3Set(&b, 0.2,  .2,  .4);
    ferVec3Add(&a, &b);
    ferVec3Set(&d, 1.5, 1.4, 3.8);
    assertTrue(ferVec3Eq(&a, &d));

    ferVec3Set(&a, 1., 1.2, 3.4);
    ferVec3Scale(&a, 2.);
    ferVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(ferVec3Eq(&a, &d));
    ferVec3Set(&a, 1.3, 1.2, 3.4);
    ferVec3Scale(&a, 0.2);
    ferVec3Set(&d, 1.3 * .2, 1.2 * .2, 3.4 * .2);
    assertTrue(ferVec3Eq(&a, &d));

    ferVec3Set(&a, 2., 1.1, 5.4);
    ferVec3Normalize(&a);
    assertTrue(ferEq(1., ferVec3Len2(&a)));
    ferVec3Set(&a, 1., .1, 3.4);
    ferVec3Normalize(&a);
    assertTrue(ferEq(1., ferVec3Len2(&a)));

    ferVec3Set(&a, 2., 1.1, 5.4);
    assertTrue(ferEq(ferVec3Len2(&a), ferVec3Dot(&a, &a)));
    ferVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(ferEq(2. + 1.1 * 1.2 + 5.4 * 3.4, ferVec3Dot(&a, &b)));
    ferVec3Set(&a, 2., 2.4, 6.8);
    assertTrue(ferEq(2. + 2.4 * 1.2 + 6.8 * 3.4, ferVec3Dot(&a, &b)));

    ferVec3Set(&a, 1., 0., 0.);
    ferVec3Set(&b, 0., 1., 0.);
    ferVec3Cross(&c, &a, &b);
    ferVec3Set(&d, 0., 0., 1.);
    assertTrue(ferVec3Eq(&c, &d));
    ferVec3Cross(&c, &b, &a);
    ferVec3Set(&d, 0., 0., -1.);
    assertTrue(ferVec3Eq(&c, &d));
    ferVec3Set(&a, 1., 1., 1.);
    ferVec3Set(&b, 0., 1., 0.);
    ferVec3Cross(&c, &a, &b);
    ferVec3Set(&d, -1., 0., 1.);
    assertTrue(ferVec3Eq(&c, &d));
    ferVec3Cross(&c, &b, &a);
    ferVec3Set(&d, 1., 0., -1.);
    assertTrue(ferVec3Eq(&c, &d));
}

TEST(vec3PointSegmentDist)
{
    fer_vec3_t P, a, b, w, ew;
    fer_real_t dist;

    ferVec3Set(&a, 0., 0., 0.);
    ferVec3Set(&b, 1., 0., 0.);

    // extereme w == a
    ferVec3Set(&P, -1., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 1.));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -0.5, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.5 * 0.5));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -0.1, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, .1 * .1));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, 0., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -1., 1., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 2.));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -0.5, 0.5, 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.5));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -0.1, -1., 2.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 5.01));
    assertTrue(ferVec3Eq(&w, &a));


    // extereme w == b
    ferVec3Set(&P, 2., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 1.));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 1.5, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.5 * 0.5));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 1.1, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, .1 * .1));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 1., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 2., 1., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 2.));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 1.5, 0.5, 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.5));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 1.1, -1., 2.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 5.01));
    assertTrue(ferVec3Eq(&w, &b));

    // inside segment
    ferVec3Set(&P, .5, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &P));

    ferVec3Set(&P, .9, 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &P));

    ferVec3Set(&P, .5, 1., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 1.));
    ferVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(ferVec3Eq(&w, &ew));

    ferVec3Set(&P, .5, 1., 1.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 2.));
    ferVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(ferVec3Eq(&w, &ew));



    ferVec3Set(&a, -.5, 2., 1.);
    ferVec3Set(&b, 1., 1.5, 0.5);

    // extereme w == a
    ferVec3Set(&P, -10., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 9.5 * 9.5 + 2. * 2. + 1.));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, -10., 9.2, 3.4);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 9.5 * 9.5 + 7.2 * 7.2 + 2.4 * 2.4));
    assertTrue(ferVec3Eq(&w, &a));

    // extereme w == b
    ferVec3Set(&P, 10., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 9. * 9. + 1.5 * 1.5 + 0.5 * 0.5));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, 10., 9.2, 3.4);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 9. * 9. + 7.7 * 7.7 + 2.9 * 2.9));
    assertTrue(ferVec3Eq(&w, &b));

    // inside ab
    ferVec3Set(&a, -.1, 1., 1.);
    ferVec3Set(&b, 1., 1., 1.);
    ferVec3Set(&P, 0., 0., 0.);
    dist = ferVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(ferEq(dist, 2.));
    ferVec3Set(&ew, 0., 1., 1.);
    assertTrue(ferVec3Eq(&w, &ew));
}


TEST(vec3PointTriDist)
{
    fer_vec3_t P, a, b, c, w, P0;
    fer_real_t dist;

    ferVec3Set(&a, -1., 0., 0.);
    ferVec3Set(&b, 0., 1., 1.);
    ferVec3Set(&c, -1., 0., 1.);

    ferVec3Set(&P, -1., 0., 0.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &a));

    ferVec3Set(&P, 0., 1., 1.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &b));

    ferVec3Set(&P, -1., 0., 1.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &c));

    ferVec3Set(&P, 0., 0., 0.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, NULL);
    assertTrue(ferEq(dist, 2./3.));


    // region 4
    ferVec3Set(&P, -2., 0., 0.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &a)));
    assertTrue(ferVec3Eq(&w, &a));
    ferVec3Set(&P, -2., 0.2, -1.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &a)));
    assertTrue(ferVec3Eq(&w, &a));

    // region 2
    ferVec3Set(&P, -1.3, 0., 1.2);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &c)));
    assertTrue(ferVec3Eq(&w, &c));
    ferVec3Set(&P, -1.2, 0.2, 1.1);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &c)));
    assertTrue(ferVec3Eq(&w, &c));

    // region 6
    ferVec3Set(&P, 0.3, 1., 1.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &b)));
    assertTrue(ferVec3Eq(&w, &b));
    ferVec3Set(&P, .1, 1., 1.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, ferVec3Dist2(&P, &b)));
    assertTrue(ferVec3Eq(&w, &b));

    // region 1
    ferVec3Set(&P, 0., 1., 2.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 1.));
    assertTrue(ferVec3Eq(&w, &b));
    ferVec3Set(&P, -1., 0., 2.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 1.));
    assertTrue(ferVec3Eq(&w, &c));
    ferVec3Set(&P, -0.5, 0.5, 2.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 1.));
    ferVec3Set(&P0, -0.5, 0.5, 1.);
    assertTrue(ferVec3Eq(&w, &P0));

    // region 3
    ferVec3Set(&P, -2., -1., 0.7);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 2.));
    ferVec3Set(&P0, -1., 0., 0.7);
    assertTrue(ferVec3Eq(&w, &P0));

    // region 5
    ferVec3Set(&P, 0., 0., 0.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 2./3.));
    ferVec3Set(&P0, -2./3., 1./3., 1./3.);
    assertTrue(ferVec3Eq(&w, &P0));

    // region 0
    ferVec3Set(&P, -0.5, 0.5, 0.5);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &P));
    ferVec3Set(&P, -0.5, 0.5, 0.7);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &P));
    ferVec3Set(&P, -0.5, 0.5, 0.9);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.));
    assertTrue(ferVec3Eq(&w, &P));

    ferVec3Set(&P, 0., 0., 0.5);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.5));
    ferVec3Set(&P0, -.5, .5, .5);
    assertTrue(ferVec3Eq(&w, &P0));

    ferVec3Set(&a, -1., 0., 0.);
    ferVec3Set(&b, 0., 1., -1.);
    ferVec3Set(&c, 0., 1., 1.);
    ferVec3Set(&P, 0., 0., 0.);
    dist = ferVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(ferEq(dist, 0.5));
    ferVec3Set(&P0, -.5, .5, 0.);
    assertTrue(ferVec3Eq(&w, &P0));
    //fprintf(stderr, "dist: %lf\n", dist);
}

TEST(vec3PointInTri)
{
    fer_vec3_t v[5];

    ferVec3Set(&v[0], 1., 1., 0.);
    ferVec3Set(&v[1], 0., 0., 0.);
    ferVec3Set(&v[2], 2., 0.5, 0.);
    ferVec3Set(&v[3], 0., 2., 0.);
    assertTrue(ferVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    ferVec3Set(&v[0], 1., 1., 0.);
    ferVec3Set(&v[1], 0., 0., 0.);
    ferVec3Set(&v[2], 2., -1., 0.);
    ferVec3Set(&v[3], 0., 2., 0.);
    assertFalse(ferVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    ferVec3Set(&v[0], 1., 1., 0.);
    ferVec3Set(&v[1], 0., 0., 0.);
    ferVec3Set(&v[2], 0., 0., 0.);
    ferVec3Set(&v[3], 0., 0., 0.);
    assertFalse(ferVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    ferVec3Set(&v[0], 1., 1., 0.);
    ferVec3Set(&v[1], 1., 0., 0.);
    ferVec3Set(&v[2], 0., 0., 0.);
    ferVec3Set(&v[3], -1., 0., 0.);
    assertFalse(ferVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    ferVec3Set(&v[4], 0., 0., 0.);
    ferVec3Set(&v[1], -1., 1., -1.);
    ferVec3Set(&v[2], 1., 1., -1.);
    ferVec3Set(&v[3], 0., 1., 2.);
    ferVec3ProjToPlane(&v[4], &v[1], &v[2], &v[3], &v[0]);
    assertTrue(ferVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    ferVec3Set(&v[1], 0., 0., 0.);
    ferVec3Set(&v[2], -1., 1., -1.);
    ferVec3Set(&v[3], 1., 0.8, -1.);
    ferVec3Set(&v[4], 0., 1.3, 2.2);
    ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]);
    assertTrue(ferVec3PointInTri(&v[0], &v[2], &v[3], &v[4]));
}

TEST(vec3Angle)
{
    fer_vec3_t v[3];
    fer_real_t a;
    size_t i;

    ferVec3Set(&v[0], 2., 2., 0.);
    ferVec3Set(&v[1], 1., 0., 0.);
    a = ferVec3Angle(&v[0], fer_vec3_origin, &v[1]);
    assertTrue(ferEq(a, M_PI_4));

    ferVec3Set(&v[0], 1., 2., 1.);
    ferVec3Set(&v[1], 0., 1., 0.);
    ferVec3Set(&v[2], -1., 0., -1.);
    a = ferVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(ferEq(a, M_PI));

    ferVec3Set(&v[0], 0., 1., 1.);
    ferVec3Set(&v[1], 0., 1., 0.);
    ferVec3Set(&v[2], -1., 1., 0.);
    a = ferVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(ferEq(a, M_PI_2));

    ferVec3Set(&v[0], 0., 1., 1.);
    ferVec3Set(&v[1], 0., 1., 0.);
    ferVec3Set(&v[2], -10., 11., 0.);
    a = ferVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(ferEq(a, M_PI_2));

    ferVec3Set(&v[0], 0., 1., 1.);
    ferVec3Set(&v[1], 0., 1., 0.);
    ferVec3Set(&v[2], 0.01, 0.06, 0.);
    a = ferVec3Angle(&v[0], &v[1], &v[2]);
    assertTrue(ferEq(a, M_PI_2));

    printf("# ---- angle ----\n");
    for (i = 0; i < vecs_len - 2; i++){
        a = ferVec3Angle(&vecs[i], &vecs[i + 1], &vecs[i + 2]);
        printf("# %g\n", a);
    }
    printf("# ---- angle end ----\n\n");
}

static void projToPlanePrint(fer_vec3_t *vs, int num)
{
    size_t i;

    printf("------\n");
    printf("Point color: 0.8 0.8 0.8\n");
    printf("Face color: 0 1 0\n");
    printf("Name: Proj %d - Plane\n", num);
    printf("Points:\n");
    for (i=0; i < 3; i++)
        printf("%g %g %g\n", ferVec3X(&vs[i + 2]), ferVec3Y(&vs[i + 2]), ferVec3Z(&vs[i + 2]));
    printf("Faces: 0 1 2\n");

    printf("------\n");
    printf("Point color: 1 0 0 \n");
    printf("Name: Proj %d - OPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", ferVec3X(&vs[1]), ferVec3Y(&vs[1]), ferVec3Z(&vs[1]));

    printf("------\n");
    printf("Point color: 0 0 1 \n");
    printf("Name: Proj %d - PPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", ferVec3X(&vs[0]), ferVec3Y(&vs[0]), ferVec3Z(&vs[0]));
}

TEST(vec3ProjToPlane)
{
    fer_vec3_t v[6];

    ferVec3Set(&v[1], 0., 0., 0.);
    ferVec3Set(&v[2], 0., 0., 0.);
    ferVec3Set(&v[3], 1., 0., 0.);
    ferVec3Set(&v[4], 0., 1., 0.);
    ferVec3Set(&v[5], 0., 0., 0.);
    assertTrue(ferEq(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]), FER_ZERO));
    assertTrue(ferVec3Eq(&v[0], &v[5]));

    ferVec3Set(&v[1], 0., 0., 1.);
    ferVec3Set(&v[2], 0., 0., 0.);
    ferVec3Set(&v[3], 1., 0., 0.);
    ferVec3Set(&v[4], 0., 1., 0.);
    ferVec3Set(&v[5], 0., 0., 0.);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    assertTrue(ferVec3Eq(&v[0], &v[5]));

    ferVec3Set(&v[1], 0., 0., -1.);
    ferVec3Set(&v[2], 0., 0., 0.);
    ferVec3Set(&v[3], 1., 0., 0.);
    ferVec3Set(&v[4], 0., 1., 0.);
    ferVec3Set(&v[5], 0., 0., 0.);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    assertTrue(ferVec3Eq(&v[0], &v[5]));

    ferVec3Set(&v[1], 0., 0., 1.);
    ferVec3Set(&v[2], -1., -1., -1.);
    ferVec3Set(&v[3], 1., -1., -1.);
    ferVec3Set(&v[4], -1., 1., -1.);
    ferVec3Set(&v[5], 0., 0., -1.);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    assertTrue(ferVec3Eq(&v[0], &v[5]));

    ferVec3Set(&v[1], 0.5, 10., 0.5);
    ferVec3Set(&v[2], 1., 0., 0.);
    ferVec3Set(&v[3], -1., 0., 1.);
    ferVec3Set(&v[4], -1., 0., -1.);
    ferVec3Set(&v[5], 0.5, 0., 0.5);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    assertTrue(ferVec3Eq(&v[0], &v[5]));

    ferVec3Set(&v[1], 0.5, 10., 0.5);
    ferVec3Set(&v[2], -1., -1., -1.);
    ferVec3Set(&v[3], 1., 1., 1.);
    ferVec3Set(&v[4], 0., 0., 0.);
    assertFalse(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);

    ferVec3Set(&v[1], 0., 0., 1.);
    ferVec3Set(&v[2], -1., -1., -1.);
    ferVec3Set(&v[3], 1., 1., 1.);
    ferVec3Set(&v[4], -1., 1., 1.);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    projToPlanePrint(v, 1);

    ferVec3Set(&v[1], 3., -2., 1.);
    ferVec3Set(&v[2], -1., -2., -1.);
    ferVec3Set(&v[3], 1., 2., 1.8);
    ferVec3Set(&v[4], -1.4, 1., 1.2);
    assertTrue(ferVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > FER_ZERO);
    projToPlanePrint(v, 2);
}


TEST(vec3Centroid)
{
    fer_vec3_t v[5];

    ferVec3Set(&v[0], 0., 0., 0.);
    ferVec3Set(&v[1], 1., 0., 0.);
    ferVec3Set(&v[2], 0., 1., 0.);
    ferVec3TriCentroid(&v[0], &v[1], &v[2], &v[3]);

    ferVec3Set(&v[4], 1./3., 1./3., 0.);
    assertTrue(ferVec3Eq(&v[3], &v[4]));
}


TEST(vec3TriTriOverlap)
{
    fer_vec3_t p1, q1, r1, p2, q2, r2;
    int res;
    size_t i;

    ferVec3Set(&p1, 0, 0, 0);
    ferVec3Set(&q1, 1, 0, 0);
    ferVec3Set(&r1, 1, 1, 0);
    ferVec3Set(&p2, 0, 0, 1);
    ferVec3Set(&q2, 1, 0, 1);
    ferVec3Set(&r2, 1, 1, 1);
    res = ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertFalse(res);

    ferVec3Set(&p1, 0, 0, 0);
    ferVec3Set(&q1, 1, 0, 0);
    ferVec3Set(&r1, 1, 1, 0);
    ferVec3Set(&p2, 0.1, 0, 0);
    ferVec3Set(&q2, 1.1, 0, 0);
    ferVec3Set(&r2, 1.1, 1, 0);
    res = ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertTrue(res);

    ferVec3Set(&p1, 0, 0, 0);
    ferVec3Set(&q1, 1, 0, 0);
    ferVec3Set(&r1, 1, 1, 0);
    ferVec3Set(&p2, 0.5, 0.5, 0.5);
    ferVec3Set(&q2, 0.5, 0.5, -1);
    ferVec3Set(&r2, -1, 0, 0);
    res = ferVec3TriTriOverlap(&p1, &q1, &r1, &p2, &q2, &r2);
    assertTrue(res);

    printf("# ---- tri tri overlap ----\n");
    for (i = 0; i < vecs_len / 100; i += 6){
        res = ferVec3TriTriOverlap(&vecs[i + 0], &vecs[i + 1], &vecs[i + 2],
                                   &vecs[i + 3], &vecs[i + 4], &vecs[i + 5]);
        printf("# res[%04u]: %d\n", i, res);
        /*
        char name[120];
        sprintf(name, "v: %d", res);
        printf("--\nName: [%04u] %s\n", i, name);
        printf("Points:\n");
        ferVec3Print(&vecs[i + 0], stdout); printf("\n");
        ferVec3Print(&vecs[i + 1], stdout); printf("\n");
        ferVec3Print(&vecs[i + 2], stdout); printf("\n");
        printf("Edges:\n0 1\n1 2\n2 0\n--\n");
        printf("---\nPoints:\n");
        ferVec3Print(&vecs[i + 3], stdout); printf("\n");
        ferVec3Print(&vecs[i + 4], stdout); printf("\n");
        ferVec3Print(&vecs[i + 5], stdout); printf("\n");
        printf("Edges:\n0 1\n1 2\n2 0\n--\n");
        */
    }
    printf("# ---- tri tri overlap end ----\n\n");
}
