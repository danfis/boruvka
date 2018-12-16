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

    TEST_ADD(vec2BoxBoxOverlap),

    TEST_ADD(vec2TearDown),
    TEST_SUITE_CLOSURE
};
#endif
