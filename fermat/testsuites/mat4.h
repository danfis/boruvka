#ifndef TEST_MAT4_H
#define TEST_MAT4_H

#include <cu/cu.h>

TEST(mat4SetUp);
TEST(mat4TearDown);
TEST(mat4Alloc);

TEST(mat4AddSub);
TEST(mat4Const);
TEST(mat4Mul);
TEST(mat4Trans);
TEST(mat4Det);
TEST(mat4Inv);
TEST(mat4MulVec);

TEST(mat4Tr);

TEST_SUITE(TSMat4) {
    TEST_ADD(mat4SetUp),

    TEST_ADD(mat4Alloc),

    TEST_ADD(mat4AddSub),
    TEST_ADD(mat4Const),
    TEST_ADD(mat4Mul),
    TEST_ADD(mat4Trans),
    TEST_ADD(mat4Det),
    TEST_ADD(mat4Inv),
    TEST_ADD(mat4MulVec),

    TEST_ADD(mat4Tr),

    TEST_ADD(mat4TearDown),
    TEST_SUITE_CLOSURE
};
#endif
