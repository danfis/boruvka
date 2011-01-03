#include <stdio.h>
#include "cu.h"

#include <fermat/vec2.h>
#include <fermat/tr2.h>

static fer_vec2_t *v[4];
static fer_vec2_t *t, *p;

TEST(vec2SetUp)
{
    int i;

    t = ferVec2New(0., 0.);
    p = ferVec2New(0., 0.);


    for (i=0; i < 4; i++)
        v[i] = ferVec2New(0., 0.);
}

TEST(vec2TearDown)
{
    int i;

    ferVec2Del(t);
    ferVec2Del(p);
    for (i=0; i < 4; i++)
        ferVec2Del(v[i]);
}

TEST(vec2Init)
{
    ferVec2Set(v[0], 1., 2.);
    ferVec2Set(v[1], 2., 3.);
    ferVec2Sub2(v[2], v[1], v[0]);
    ferVec2Sub2(v[3], v[1], v[0]);
    ferVec2Scale(v[3], 2.);

    ferVec2Set(t, 1., 2.);
    assertTrue(ferVec2Eq(v[0], t));

    ferVec2Set(t, 2., 3.);
    assertTrue(ferVec2Eq(v[1], t));

    ferVec2Set(t, 1., 1.);
    assertTrue(ferVec2Eq(v[2], t));

    ferVec2Set(t, 2., 2.);
    assertTrue(ferVec2Eq(v[3], t));


    ferVec2Copy(v[0], v[1]);
    ferVec2Set(t, 2., 3.);
    assertTrue(ferVec2Eq(v[0], t));
    assertTrue(ferVec2Eq(v[0], v[1]));

    ferVec2Sub2(v[0], v[1], v[2]);
    ferVec2Set(t, 1., 2.);
    assertTrue(ferVec2Eq(v[0], t));
}

TEST(vec2Clone)
{
    fer_vec2_t *vc = ferVec2Clone(v[0]);
    assertTrue(ferVec2Eq(vc, v[0]));
    ferVec2Del(vc);
}


TEST(vec2Operators)
{
    ferVec2Set(v[0], 1., 2.);
    ferVec2Set(v[1], 1., 2.);
    ferVec2Set(v[2], 2., 4.);
    ferVec2Set(v[3], 0., 0.);

    assertTrue(ferVec2Eq(v[0], v[1]));
    assertTrue(ferVec2NEq(v[0], v[2]));

    ferVec2Add(v[0], v[1]);
    assertTrue(ferVec2Eq(v[2], v[0]));

    ferVec2Sub(v[0], v[2]);
    assertTrue(ferVec2Eq(v[0], v[3]));
}

TEST(vec2Length)
{
    ferVec2Set(v[0], 10., 0.);
    ferVec2Set(v[1], 6., 4.);
    ferVec2Set(v[2], 1., 1.);

    assertTrue(ferEq(FER_SQRT(ferVec2Len2(v[0])), 10.));
    assertTrue(ferEq(ferVec2Len2(v[1]), FER_REAL(52.)));
    assertTrue(ferEq(ferVec2Dist2(v[1], v[2]), FER_REAL(34.)));

    assertTrue(ferEq(ferVec2Dist2(v[1], v[2]), ferVec2Dist2(v[2], v[1])));

    assertTrue(ferVec2Eq(v[0], v[0]));
}

TEST(vec2AngleArea)
{
    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 1., 1.);
    ferVec2Set(v[2], 0., 1.);

    assertTrue(ferEq(ferVec2Angle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(ferEq(ferVec2Angle(v[2], v[0], v[1]), M_PI_4));
    assertTrue(ferEq(ferVec2Angle(v[0], v[1], v[2]), M_PI_4));
    assertTrue(ferEq(ferVec2Angle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(ferEq(ferVec2Angle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(ferEq(ferVec2Angle(v[1], v[2], v[0]), M_PI_2));

    assertTrue(ferEq(ferVec2SignedAngle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(ferEq(ferVec2SignedAngle(v[2], v[0], v[1]), -1. * M_PI_4));
    assertTrue(ferEq(ferVec2SignedAngle(v[0], v[1], v[2]), -1. * M_PI_4));
    assertTrue(ferEq(ferVec2SignedAngle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(ferEq(ferVec2SignedAngle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(ferEq(ferVec2SignedAngle(v[1], v[2], v[0]), -1. * M_PI_2));


    assertTrue(ferEq(ferVec2Area2(v[0], v[1], v[2]), 1));
    assertTrue(ferEq(ferVec2Area2(v[1], v[2], v[0]), 1));
    assertTrue(ferEq(ferVec2Area2(v[2], v[0], v[1]), 1));

    assertTrue(ferEq(ferVec2Area2(v[2], v[1], v[0]), -1));
    assertTrue(ferEq(ferVec2Area2(v[1], v[0], v[2]), -1));
    assertTrue(ferEq(ferVec2Area2(v[0], v[2], v[1]), -1));
}

TEST(vec2Intersect)
{
    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 6., 0.);
    ferVec2Set(v[2], 3., -3.);
    ferVec2Set(v[3], 3., 3.);

    assertEquals(ferVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    ferVec2Set(t, 3., 0.);
    assertTrue(ferVec2Eq(p, t));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 6., 0.);
    ferVec2Set(v[2], 10., 10.);
    ferVec2Set(v[3], 3., 3.);
    assertNotEquals(ferVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 3., 3.);
    ferVec2Set(v[2], -1., 0.);
    ferVec2Set(v[3], 2., 2.);
    assertEquals(ferVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    ferVec2Set(t, 2., 2.);
    assertTrue(ferVec2Eq(p, t));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 3., 3.);
    ferVec2Set(v[2], -1., 0.);
    ferVec2Set(v[3], 4., 3.);
    assertEquals(ferVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    ferVec2Set(t, 1.5, 1.5);
    assertTrue(ferVec2Eq(t, p));
}

TEST(vec2SegmentInRect)
{
    fer_vec2_t *s[2], *l[2];

    s[0] = ferVec2New(0., 0.);
    s[1] = ferVec2New(0., 0.);
    l[0] = ferVec2New(0., 0.);
    l[1] = ferVec2New(0., 0.);

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 0.);
    ferVec2Set(v[2], 2., 2.);
    ferVec2Set(v[3], 0., 2.);

    ferVec2Set(l[0], 1., 1.);
    ferVec2Set(l[1], 3., 3.);
    assertEquals(ferVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((ferVec2Eq2(s[0], 1., 1.) && ferVec2Eq2(s[1], 2., 2.))
            ||
        (ferVec2Eq2(s[0], 2., 2.) && ferVec2Eq2(s[1], 1., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    ferVec2Set(l[0], -1., 1.);
    ferVec2Set(l[1], 3., 1.);
    assertEquals(ferVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((ferVec2Eq2(s[0], 0., 1.) && ferVec2Eq2(s[1], 2., 1.))
            ||
        (ferVec2Eq2(s[0], 2., 1.) && ferVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
        printf("[%f %f] - [%f %f]\n", ferVec2X(s[0]), ferVec2Y(s[1]),
                ferVec2X(s[1]), ferVec2Y(s[1]));
    }

    ferVec2Set(l[0], -1., 1.);
    ferVec2Set(l[1], 1., 1.);
    assertEquals(ferVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((ferVec2Eq2(s[0], 0., 1.) && ferVec2Eq2(s[1], 1., 1.))
            ||
        (ferVec2Eq2(s[0], 1., 1.) && ferVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    ferVec2Set(l[0], 0.5, 1.5);
    ferVec2Set(l[1], 1.5, 0.5);
    assertEquals(ferVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((ferVec2Eq2(s[0], 0.5, 1.5) && ferVec2Eq2(s[1], 1.5, 0.5))
            ||
        (ferVec2Eq2(s[0], 1.5, 0.5) && ferVec2Eq2(s[1], 0.5, 1.5))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    ferVec2Set(l[0], 0., 1.);
    ferVec2Set(l[1], 2., 1.);
    assertEquals(ferVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((ferVec2Eq2(s[0], 0., 1.) && ferVec2Eq2(s[1], 2., 1.))
            ||
        (ferVec2Eq2(s[0], 2., 1.) && ferVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    ferVec2Del(s[0]);
    ferVec2Del(s[1]);
    ferVec2Del(l[0]);
    ferVec2Del(l[1]);
}

TEST(vec2Projection)
{
    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 0., 2.);
    assertEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(ferVec2Eq2(p, 1., 1.));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 3., 3.);
    assertNotEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 3., 4.);
    assertNotEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 0., 0.);
    assertEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(ferVec2Eq2(p, 0., 0.));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 2., 2.);
    assertEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(ferVec2Eq2(p, 2., 2.));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 2., 2.);
    ferVec2Set(v[2], 1., 1.);
    assertEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(ferVec2Eq2(p, 1., 1.));

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], -2., -2.);
    ferVec2Set(v[2], 0., -2.);
    assertEquals(ferVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(ferVec2Eq2(p, -1., -1.));
}

TEST(vec2AngleSameDir)
{
    fer_real_t angle;
    fer_tr2_t *tr = ferTr2New();

    ferVec2Set(v[0], 1., 1.);
    ferVec2Set(v[1], 2., 2.);
    angle = ferVec2AngleSameDir(v[0], v[1]);
    assertTrue(ferEq(angle, 0.));
    angle = ferVec2AngleSameDir(v[1], v[0]);
    assertTrue(ferEq(angle, 0.));

    ferVec2Set(v[0], 0., 1.);
    ferVec2Set(v[1], 1., 0.);
    angle = ferVec2AngleSameDir(v[0], v[1]);
    assertTrue(ferEq(angle, M_PI_2));
    angle = ferVec2AngleSameDir(v[1], v[0]);
    assertTrue(ferEq(angle, -1. * M_PI_2));


    // --- Oriented segments:
    printf("# vecAngleSameDir - Oriented segments\n");
    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 1., 1.);
    ferVec2Set(v[2], 0., 0.);
    ferVec2Set(v[3], 2., 2.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(ferEq(angle, 0.));


    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 1., 1.);
    ferVec2Set(v[2], 2., 0.);
    ferVec2Set(v[3], 2., 2.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(ferEq(angle, -1. * M_PI_4));

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[0]), ferVec2Y(v[0]));
    printf("%f %f\n", ferVec2X(v[1]), ferVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");
    ferTr2Identity(tr);
    ferTr2Rotate(tr, angle);
    ferTr2(tr, v[2]);
    ferTr2(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");

    ferVec2Set(v[0], 0., 0.);
    ferVec2Set(v[1], 1., 1.);
    ferVec2Set(v[2], 2., 0.);
    ferVec2Set(v[3], 2., 2.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);
    assertTrue(ferEq(angle, M_PI_4 + M_PI_2));


    ferVec2Set(v[0], 2., 3.);
    ferVec2Set(v[1], 3., 4.);
    ferVec2Set(v[2], 1., 1.);
    ferVec2Set(v[3], 3., 7.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[0]), ferVec2Y(v[0]));
    printf("%f %f\n", ferVec2X(v[1]), ferVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");
    ferTr2Identity(tr);
    ferTr2Rotate(tr, angle);
    ferTr2(tr, v[2]);
    ferTr2(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");

    ferVec2Sub2(t, v[1], v[0]);
    ferVec2Scale(t, 1./FER_SQRT(ferVec2Len2(t)));
    ferVec2Sub2(p, v[3], v[2]);
    ferVec2Scale(p, 1./FER_SQRT(ferVec2Len2(p)));
    assertTrue(ferVec2Eq(t, p));


    ferVec2Set(v[0], 2., 3.);
    ferVec2Set(v[1], 2., 4.);
    ferVec2Set(v[2], 1., 1.);
    ferVec2Set(v[3], 1., 7.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[0]), ferVec2Y(v[0]));
    printf("%f %f\n", ferVec2X(v[1]), ferVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");
    ferTr2Identity(tr);
    ferTr2Rotate(tr, angle);
    ferTr2(tr, v[2]);
    ferTr2(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");

    ferVec2Sub2(t, v[1], v[0]);
    ferVec2Scale(t, 1./FER_SQRT(ferVec2Len2(t)));
    ferVec2Sub2(p, v[2], v[3]);
    ferVec2Scale(p, 1./FER_SQRT(ferVec2Len2(p)));
    assertTrue(ferVec2Eq(t, p));


    ferVec2Set(v[0], 1., 0.);
    ferVec2Set(v[1], 1., -1.);
    ferVec2Set(v[2], -3., 1.);
    ferVec2Set(v[3], 2., 2.);
    angle = ferVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[0]), ferVec2Y(v[0]));
    printf("%f %f\n", ferVec2X(v[1]), ferVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");
    ferTr2Identity(tr);
    ferTr2Rotate(tr, angle);
    ferTr2(tr, v[2]);
    ferTr2(tr, v[3]);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", ferVec2X(v[2]), ferVec2Y(v[2]));
    printf("%f %f\n", ferVec2X(v[3]), ferVec2Y(v[3]));
    printf("----\n");

    ferVec2Sub2(t, v[0], v[1]);
    ferVec2Scale(t, 1./FER_SQRT(ferVec2Len2(t)));
    ferVec2Sub2(p, v[2], v[3]);
    ferVec2Scale(p, 1./FER_SQRT(ferVec2Len2(p)));
    assertTrue(ferVec2Eq(t, p));


    ferTr2Del(tr);
}
