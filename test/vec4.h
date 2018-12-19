#ifndef TEST_VEC4_H
#define TEST_VEC4_H

#include <cu/cu.h>

TEST(vec4SetUp);
TEST(vec4TearDown);
TEST(vec4Alloc);

TEST_SUITE(TSVec4) {
    TEST_ADD(vec4SetUp),

    TEST_ADD(vec4Alloc),

    TEST_ADD(vec4TearDown),
    TEST_SUITE_CLOSURE
};
#endif
