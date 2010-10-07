#include <stdio.h>
#include <cu/cu.h>
#include <mg/vec3.h>

TEST(vec3SetUp)
{
}

TEST(vec3TearDown)
{
}


TEST(vec3PointSegmentDist)
{
    mg_vec3_t P, a, b, w, ew;
    mg_real_t dist;

    mgVec3Set(&a, 0., 0., 0.);
    mgVec3Set(&b, 1., 0., 0.);

    // extereme w == a
    mgVec3Set(&P, -1., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.1, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, .1 * .1));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -1., 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.5, 0.5, 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.1, -1., 2.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 5.01));
    assertTrue(mgVec3Eq(&w, &a));


    // extereme w == b
    mgVec3Set(&P, 2., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.1, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, .1 * .1));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 2., 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.5, 0.5, 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.1, -1., 2.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 5.01));
    assertTrue(mgVec3Eq(&w, &b));

    // inside segment
    mgVec3Set(&P, .5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, .9, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, .5, 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    mgVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(mgVec3Eq(&w, &ew));

    mgVec3Set(&P, .5, 1., 1.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(mgVec3Eq(&w, &ew));



    mgVec3Set(&a, -.5, 2., 1.);
    mgVec3Set(&b, 1., 1.5, 0.5);

    // extereme w == a
    mgVec3Set(&P, -10., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9.5 * 9.5 + 2. * 2. + 1.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -10., 9.2, 3.4);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9.5 * 9.5 + 7.2 * 7.2 + 2.4 * 2.4));
    assertTrue(mgVec3Eq(&w, &a));

    // extereme w == b
    mgVec3Set(&P, 10., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9. * 9. + 1.5 * 1.5 + 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 10., 9.2, 3.4);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9. * 9. + 7.7 * 7.7 + 2.9 * 2.9));
    assertTrue(mgVec3Eq(&w, &b));

    // inside ab
    mgVec3Set(&a, -.1, 1., 1.);
    mgVec3Set(&b, 1., 1., 1.);
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&ew, 0., 1., 1.);
    assertTrue(mgVec3Eq(&w, &ew));
}


TEST(vec3PointTriDist)
{
    mg_vec3_t P, a, b, c, w, P0;
    mg_real_t dist;

    mgVec3Set(&a, -1., 0., 0.);
    mgVec3Set(&b, 0., 1., 1.);
    mgVec3Set(&c, -1., 0., 1.);

    mgVec3Set(&P, -1., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, 0., 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, -1., 0., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &c));

    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, NULL);
    assertTrue(mgEq(dist, 2./3.));


    // region 4
    mgVec3Set(&P, -2., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &a)));
    assertTrue(mgVec3Eq(&w, &a));
    mgVec3Set(&P, -2., 0.2, -1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &a)));
    assertTrue(mgVec3Eq(&w, &a));

    // region 2
    mgVec3Set(&P, -1.3, 0., 1.2);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &c)));
    assertTrue(mgVec3Eq(&w, &c));
    mgVec3Set(&P, -1.2, 0.2, 1.1);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &c)));
    assertTrue(mgVec3Eq(&w, &c));

    // region 6
    mgVec3Set(&P, 0.3, 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &b)));
    assertTrue(mgVec3Eq(&w, &b));
    mgVec3Set(&P, .1, 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &b)));
    assertTrue(mgVec3Eq(&w, &b));

    // region 1
    mgVec3Set(&P, 0., 1., 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &b));
    mgVec3Set(&P, -1., 0., 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &c));
    mgVec3Set(&P, -0.5, 0.5, 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    mgVec3Set(&P0, -0.5, 0.5, 1.);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 3
    mgVec3Set(&P, -2., -1., 0.7);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&P0, -1., 0., 0.7);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 5
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 2./3.));
    mgVec3Set(&P0, -2./3., 1./3., 1./3.);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 0
    mgVec3Set(&P, -0.5, 0.5, 0.5);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));
    mgVec3Set(&P, -0.5, 0.5, 0.7);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));
    mgVec3Set(&P, -0.5, 0.5, 0.9);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, 0., 0., 0.5);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.5));
    mgVec3Set(&P0, -.5, .5, .5);
    assertTrue(mgVec3Eq(&w, &P0));

    mgVec3Set(&a, -1., 0., 0.);
    mgVec3Set(&b, 0., 1., -1.);
    mgVec3Set(&c, 0., 1., 1.);
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.5));
    mgVec3Set(&P0, -.5, .5, 0.);
    assertTrue(mgVec3Eq(&w, &P0));
    //fprintf(stderr, "dist: %lf\n", dist);
}
