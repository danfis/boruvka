#ifndef TEST_MAT_H
#define TEST_MAT_H

#include "cu.h"

TEST(matSetUp);
TEST(matTearDown);

TEST(matInit);
TEST(matOperators);

TEST_SUITE(TSMat)
{
    TEST_ADD(matSetUp),

    TEST_ADD(matInit),
    TEST_ADD(matOperators),

    TEST_ADD(matTearDown),
    TEST_SUITE_CLOSURE
};
#endif

