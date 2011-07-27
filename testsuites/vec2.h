#ifndef TEST_VEC2_H
#define TEST_VEC2_H

#include "cu.h"

TEST(vec2SetUp);
TEST(vec2TearDown);

TEST(vec2Init);
TEST(vec2Clone);
TEST(vec2Operators);
TEST(vec2Length);
TEST(vec2AngleArea);
TEST(vec2Intersect);
TEST(vec2SegmentInRect);
TEST(vec2Projection);
TEST(vec2AngleSameDir);

TEST(vec2Add);
TEST(vec2Sub);
TEST(vec2Scale);
TEST(vec2Normalize);
TEST(vec2Dot);
TEST(vec2Mul);
TEST(vec2Len);
TEST(vec2Dist);
TEST(vec2Area);
TEST(vec2Angle);
TEST(vec2ProjSeg);
TEST(vec2InCircle);
TEST(vec2LiesOn);
TEST(vec2Collinear);
TEST(vec2InCone);
TEST(vec2Inter);
TEST(vec2OnLeft);
TEST(vec2SegInRect);

TEST(vec2BoxBoxOverlap);

TEST_SUITE(TSVec2)
{
    TEST_ADD(vec2SetUp),

    TEST_ADD(vec2Init),
    TEST_ADD(vec2Clone),
    TEST_ADD(vec2Operators),
    TEST_ADD(vec2Length),
    TEST_ADD(vec2AngleArea),
    TEST_ADD(vec2Intersect),
    TEST_ADD(vec2SegmentInRect),
    TEST_ADD(vec2Projection),
    TEST_ADD(vec2AngleSameDir),

    TEST_ADD(vec2Add),
    TEST_ADD(vec2Sub),
    TEST_ADD(vec2Scale),
    TEST_ADD(vec2Normalize),
    TEST_ADD(vec2Dot),
    TEST_ADD(vec2Mul),
    TEST_ADD(vec2Len),
    TEST_ADD(vec2Dist),
    TEST_ADD(vec2Area),
    TEST_ADD(vec2Angle),
    TEST_ADD(vec2ProjSeg),
    TEST_ADD(vec2InCircle),
    TEST_ADD(vec2LiesOn),
    TEST_ADD(vec2Collinear),
    TEST_ADD(vec2InCone),
    TEST_ADD(vec2Inter),
    TEST_ADD(vec2OnLeft),
    TEST_ADD(vec2SegInRect),

    TEST_ADD(vec2BoxBoxOverlap),

    TEST_ADD(vec2TearDown),
    TEST_SUITE_CLOSURE
};
#endif
