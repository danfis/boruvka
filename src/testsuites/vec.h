#ifndef TEST_VEC_H
#define TEST_VEC_H

#include "cu.h"

TEST(vecSetUp);
TEST(vecTearDown);

TEST(vecInit);
TEST(vecOperators);

TEST_SUITE(TSVec)
{
    TEST_ADD(vecSetUp),

    TEST_ADD(vecInit),
    TEST_ADD(vecOperators),

    TEST_ADD(vecTearDown),
    TEST_SUITE_CLOSURE
};
#endif

