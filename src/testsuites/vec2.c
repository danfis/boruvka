#include <stdio.h>
#include "cu.h"

#include <mg/vec2.h>
//#include "../tr.h"

static mg_vec2_t *v[4];
static mg_vec2_t *t, *p;

TEST(vec2SetUp)
{
    int i;

    t = mgVec2New(0., 0.);
    p = mgVec2New(0., 0.);


    for (i=0; i < 4; i++)
        v[i] = mgVec2New(0., 0.);
}

TEST(vec2TearDown)
{
    int i;

    mgVec2Del(t);
    mgVec2Del(p);
    for (i=0; i < 4; i++)
        mgVec2Del(v[i]);
}

TEST(vec2Init)
{
    mgVec2Set(v[0], 1., 2.);
    mgVec2Set(v[1], 2., 3.);
    mgVec2Sub2(v[2], v[1], v[0]);
    mgVec2Sub2(v[3], v[1], v[0]);
    mgVec2Scale(v[3], 2.);

    mgVec2Set(t, 1., 2.);
    assertTrue(mgVec2Eq(v[0], t));

    mgVec2Set(t, 2., 3.);
    assertTrue(mgVec2Eq(v[1], t));

    mgVec2Set(t, 1., 1.);
    assertTrue(mgVec2Eq(v[2], t));

    mgVec2Set(t, 2., 2.);
    assertTrue(mgVec2Eq(v[3], t));


    mgVec2Copy(v[0], v[1]);
    mgVec2Set(t, 2., 3.);
    assertTrue(mgVec2Eq(v[0], t));
    assertTrue(mgVec2Eq(v[0], v[1]));

    mgVec2Sub2(v[0], v[1], v[2]);
    mgVec2Set(t, 1., 2.);
    assertTrue(mgVec2Eq(v[0], t));
}

TEST(vec2Clone)
{
    mg_vec2_t *vc = mgVec2Clone(v[0]);
    assertTrue(mgVec2Eq(vc, v[0]));
    mgVec2Del(vc);
}


TEST(vec2Operators)
{
    mgVec2Set(v[0], 1., 2.);
    mgVec2Set(v[1], 1., 2.);
    mgVec2Set(v[2], 2., 4.);
    mgVec2Set(v[3], 0., 0.);

    assertTrue(mgVec2Eq(v[0], v[1]));
    assertTrue(mgVec2NEq(v[0], v[2]));

    mgVec2Add(v[0], v[1]);
    assertTrue(mgVec2Eq(v[2], v[0]));

    mgVec2Sub(v[0], v[2]);
    assertTrue(mgVec2Eq(v[0], v[3]));
}

TEST(vec2Length)
{
    mgVec2Set(v[0], 10., 0.);
    mgVec2Set(v[1], 6., 4.);
    mgVec2Set(v[2], 1., 1.);

    assertTrue(mgEq(MG_SQRT(mgVec2Len2(v[0])), 10.));
    assertTrue(mgEq(mgVec2Len2(v[1]), MG_REAL(52.)));
    assertTrue(mgEq(mgVec2Dist2(v[1], v[2]), MG_REAL(34.)));

    assertTrue(mgEq(mgVec2Dist2(v[1], v[2]), mgVec2Dist2(v[2], v[1])));

    assertTrue(mgVec2Eq(v[0], v[0]));
}

TEST(vec2AngleArea)
{
    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 1., 1.);
    mgVec2Set(v[2], 0., 1.);

    assertTrue(mgEq(mgVec2Angle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(mgEq(mgVec2Angle(v[2], v[0], v[1]), M_PI_4));
    assertTrue(mgEq(mgVec2Angle(v[0], v[1], v[2]), M_PI_4));
    assertTrue(mgEq(mgVec2Angle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(mgEq(mgVec2Angle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(mgEq(mgVec2Angle(v[1], v[2], v[0]), M_PI_2));

    assertTrue(mgEq(mgVec2SignedAngle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(mgEq(mgVec2SignedAngle(v[2], v[0], v[1]), -1. * M_PI_4));
    assertTrue(mgEq(mgVec2SignedAngle(v[0], v[1], v[2]), -1. * M_PI_4));
    assertTrue(mgEq(mgVec2SignedAngle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(mgEq(mgVec2SignedAngle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(mgEq(mgVec2SignedAngle(v[1], v[2], v[0]), -1. * M_PI_2));


    assertTrue(mgEq(mgVec2Area2(v[0], v[1], v[2]), 1));
    assertTrue(mgEq(mgVec2Area2(v[1], v[2], v[0]), 1));
    assertTrue(mgEq(mgVec2Area2(v[2], v[0], v[1]), 1));

    assertTrue(mgEq(mgVec2Area2(v[2], v[1], v[0]), -1));
    assertTrue(mgEq(mgVec2Area2(v[1], v[0], v[2]), -1));
    assertTrue(mgEq(mgVec2Area2(v[0], v[2], v[1]), -1));
}

TEST(vec2Intersect)
{
    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 6., 0.);
    mgVec2Set(v[2], 3., -3.);
    mgVec2Set(v[3], 3., 3.);

    assertEquals(mgVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    mgVec2Set(t, 3., 0.);
    assertTrue(mgVec2Eq(p, t));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 6., 0.);
    mgVec2Set(v[2], 10., 10.);
    mgVec2Set(v[3], 3., 3.);
    assertNotEquals(mgVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 3., 3.);
    mgVec2Set(v[2], -1., 0.);
    mgVec2Set(v[3], 2., 2.);
    assertEquals(mgVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    mgVec2Set(t, 2., 2.);
    assertTrue(mgVec2Eq(p, t));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 3., 3.);
    mgVec2Set(v[2], -1., 0.);
    mgVec2Set(v[3], 4., 3.);
    assertEquals(mgVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    mgVec2Set(t, 1.5, 1.5);
    assertTrue(mgVec2Eq(t, p));
}

TEST(vec2SegmentInRect)
{
    mg_vec2_t *s[2], *l[2];

    s[0] = mgVec2New(0., 0.);
    s[1] = mgVec2New(0., 0.);
    l[0] = mgVec2New(0., 0.);
    l[1] = mgVec2New(0., 0.);

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 0.);
    mgVec2Set(v[2], 2., 2.);
    mgVec2Set(v[3], 0., 2.);

    mgVec2Set(l[0], 1., 1.);
    mgVec2Set(l[1], 3., 3.);
    assertEquals(mgVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((mgVec2Eq2(s[0], 1., 1.) && mgVec2Eq2(s[1], 2., 2.))
            ||
        (mgVec2Eq2(s[0], 2., 2.) && mgVec2Eq2(s[1], 1., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    mgVec2Set(l[0], -1., 1.);
    mgVec2Set(l[1], 3., 1.);
    assertEquals(mgVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((mgVec2Eq2(s[0], 0., 1.) && mgVec2Eq2(s[1], 2., 1.))
            ||
        (mgVec2Eq2(s[0], 2., 1.) && mgVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
        printf("[%f %f] - [%f %f]\n", mgVec2X(s[0]), mgVec2Y(s[1]),
                mgVec2X(s[1]), mgVec2Y(s[1]));
    }

    mgVec2Set(l[0], -1., 1.);
    mgVec2Set(l[1], 1., 1.);
    assertEquals(mgVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((mgVec2Eq2(s[0], 0., 1.) && mgVec2Eq2(s[1], 1., 1.))
            ||
        (mgVec2Eq2(s[0], 1., 1.) && mgVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    mgVec2Set(l[0], 0.5, 1.5);
    mgVec2Set(l[1], 1.5, 0.5);
    assertEquals(mgVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((mgVec2Eq2(s[0], 0.5, 1.5) && mgVec2Eq2(s[1], 1.5, 0.5))
            ||
        (mgVec2Eq2(s[0], 1.5, 0.5) && mgVec2Eq2(s[1], 0.5, 1.5))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    mgVec2Set(l[0], 0., 1.);
    mgVec2Set(l[1], 2., 1.);
    assertEquals(mgVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((mgVec2Eq2(s[0], 0., 1.) && mgVec2Eq2(s[1], 2., 1.))
            ||
        (mgVec2Eq2(s[0], 2., 1.) && mgVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    mgVec2Del(s[0]);
    mgVec2Del(s[1]);
    mgVec2Del(l[0]);
    mgVec2Del(l[1]);
}

TEST(vec2Projection)
{
    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 0., 2.);
    assertEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(mgVec2Eq2(p, 1., 1.));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 3., 3.);
    assertNotEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 3., 4.);
    assertNotEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 0., 0.);
    assertEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(mgVec2Eq2(p, 0., 0.));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 2., 2.);
    assertEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(mgVec2Eq2(p, 2., 2.));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 2., 2.);
    mgVec2Set(v[2], 1., 1.);
    assertEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(mgVec2Eq2(p, 1., 1.));

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], -2., -2.);
    mgVec2Set(v[2], 0., -2.);
    assertEquals(mgVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(mgVec2Eq2(p, -1., -1.));
}

TEST(vec2AngleSameDir)
{
    /*
    mg_real_t angle;
    g_tr_t *tr = gTrNew();

    mgVec2Set(v[0], 1., 1.);
    mgVec2Set(v[1], 2., 2.);
    angle = mgVec2AngleSameDir(v[0], v[1]);
    assertTrue(mgEq(angle, 0.));
    angle = mgVec2AngleSameDir(v[1], v[0]);
    assertTrue(mgEq(angle, 0.));

    mgVec2Set(v[0], 0., 1.);
    mgVec2Set(v[1], 1., 0.);
    angle = mgVec2AngleSameDir(v[0], v[1]);
    assertTrue(mgEq(angle, M_PI_2));
    angle = mgVec2AngleSameDir(v[1], v[0]);
    assertTrue(mgEq(angle, -1. * M_PI_2));


    // --- Oriented segments:
    printf("# vecAngleSameDir - Oriented segments\n");
    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 1., 1.);
    mgVec2Set(v[2], 0., 0.);
    mgVec2Set(v[3], 2., 2.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(mgEq(angle, 0.));


    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 1., 1.);
    mgVec2Set(v[2], 2., 0.);
    mgVec2Set(v[3], 2., 2.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(mgEq(angle, -1. * M_PI_4));

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[0]), mgVec2Y(v[0]));
    printf("%f %f\n", mgVec2X(v[1]), mgVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");
    gTrIdentity(tr);
    gTrRotate(tr, angle);
    gTr(tr, v[2]);
    gTr(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");

    mgVec2Set(v[0], 0., 0.);
    mgVec2Set(v[1], 1., 1.);
    mgVec2Set(v[2], 2., 0.);
    mgVec2Set(v[3], 2., 2.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);
    assertTrue(mgEq(angle, M_PI_4 + M_PI_2));


    mgVec2Set(v[0], 2., 3.);
    mgVec2Set(v[1], 3., 4.);
    mgVec2Set(v[2], 1., 1.);
    mgVec2Set(v[3], 3., 7.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[0]), mgVec2Y(v[0]));
    printf("%f %f\n", mgVec2X(v[1]), mgVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");
    gTrIdentity(tr);
    gTrRotate(tr, angle);
    gTr(tr, v[2]);
    gTr(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");

    mgVec2SetVec(t, v[1], v[0]);
    mgVec2Scale(t, 1./mgVec2Length(t));
    mgVec2SetVec(p, v[3], v[2]);
    mgVec2Scale(p, 1./mgVec2Length(p));
    assertTrue(mgVec2Eq(t, p));


    mgVec2Set(v[0], 2., 3.);
    mgVec2Set(v[1], 2., 4.);
    mgVec2Set(v[2], 1., 1.);
    mgVec2Set(v[3], 1., 7.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[0]), mgVec2Y(v[0]));
    printf("%f %f\n", mgVec2X(v[1]), mgVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");
    gTrIdentity(tr);
    gTrRotate(tr, angle);
    gTr(tr, v[2]);
    gTr(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");

    mgVec2SetVec(t, v[1], v[0]);
    mgVec2Scale(t, 1./mgVec2Length(t));
    mgVec2SetVec(p, v[2], v[3]);
    mgVec2Scale(p, 1./mgVec2Length(p));
    assertTrue(mgVec2Eq(t, p));


    mgVec2Set(v[0], 1., 0.);
    mgVec2Set(v[1], 1., -1.);
    mgVec2Set(v[2], -3., 1.);
    mgVec2Set(v[3], 2., 2.);
    angle = mgVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[0]), mgVec2Y(v[0]));
    printf("%f %f\n", mgVec2X(v[1]), mgVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");
    gTrIdentity(tr);
    gTrRotate(tr, angle);
    gTr(tr, v[2]);
    gTr(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", mgVec2X(v[2]), mgVec2Y(v[2]));
    printf("%f %f\n", mgVec2X(v[3]), mgVec2Y(v[3]));
    printf("----\n");

    mgVec2SetVec(t, v[0], v[1]);
    mgVec2Scale(t, 1./mgVec2Length(t));
    mgVec2SetVec(p, v[2], v[3]);
    mgVec2Scale(p, 1./mgVec2Length(p));
    assertTrue(mgVec2Eq(t, p));


    gTrDel(tr);
    */
}
