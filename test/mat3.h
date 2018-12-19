#ifndef TEST_MAT3_H
#define TEST_MAT3_H

#include <cu/cu.h>

TEST(mat3SetUp);
TEST(mat3TearDown);
TEST(mat3Alloc);

TEST(mat3Tr);

TEST_SUITE(TSMat3) {
    TEST_ADD(mat3SetUp),

    TEST_ADD(mat3Alloc),

    TEST_ADD(mat3Tr),

    TEST_ADD(mat3TearDown),
    TEST_SUITE_CLOSURE
};
#endif
