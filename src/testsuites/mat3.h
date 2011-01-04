#ifndef TEST_MAT3_H
#define TEST_MAT3_H

#include <cu/cu.h>

TEST(mat3SetUp);
TEST(mat3TearDown);
TEST(mat3Alloc);

TEST(mat3AddSub);
TEST(mat3Const);
TEST(mat3Mul);
TEST(mat3Trans);
TEST(mat3Det);
TEST(mat3Inv);
TEST(mat3MulVec);

TEST_SUITE(TSMat3) {
    TEST_ADD(mat3SetUp),

    TEST_ADD(mat3Alloc),

    TEST_ADD(mat3AddSub),
    TEST_ADD(mat3Const),
    TEST_ADD(mat3Mul),
    TEST_ADD(mat3Trans),
    TEST_ADD(mat3Det),
    TEST_ADD(mat3Inv),
    TEST_ADD(mat3MulVec),

    TEST_ADD(mat3TearDown),
    TEST_SUITE_CLOSURE
};
#endif
