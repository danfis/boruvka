#ifndef TEST_VEC4_H
#define TEST_VEC4_H

#include <cu/cu.h>

TEST(vec4SetUp);
TEST(vec4TearDown);
TEST(vec4Alloc);

TEST(vec4Add);
TEST(vec4Sub);
TEST(vec4Scale);
TEST(vec4Normalize);
TEST(vec4Dot);
TEST(vec4Mul);
TEST(vec4Len2);
TEST(vec4Len);
TEST(vec4Dist2);
TEST(vec4Dist);

TEST_SUITE(TSVec4) {
    TEST_ADD(vec4SetUp),

    TEST_ADD(vec4Alloc),

    TEST_ADD(vec4Add),
    TEST_ADD(vec4Sub),
    TEST_ADD(vec4Scale),
    TEST_ADD(vec4Normalize),
    TEST_ADD(vec4Dot),
    TEST_ADD(vec4Mul),
    TEST_ADD(vec4Len2),
    TEST_ADD(vec4Len),
    TEST_ADD(vec4Dist2),
    TEST_ADD(vec4Dist),

    TEST_ADD(vec4TearDown),
    TEST_SUITE_CLOSURE
};
#endif
