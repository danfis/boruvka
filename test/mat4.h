#ifndef TEST_MAT4_H
#define TEST_MAT4_H

#include <cu/cu.h>

TEST(mat4SetUp);
TEST(mat4TearDown);
TEST(mat4Alloc);

TEST(mat4Tr);

TEST_SUITE(TSMat4) {
    TEST_ADD(mat4SetUp),

    TEST_ADD(mat4Alloc),

    TEST_ADD(mat4Tr),

    TEST_ADD(mat4TearDown),
    TEST_SUITE_CLOSURE
};
#endif
