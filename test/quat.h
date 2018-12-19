#ifndef TEST_QUAT_H
#define TEST_QUAT_H

#include <cu/cu.h>

TEST(quatCore);

TEST_SUITE(TSQuat) {
    TEST_ADD(quatCore),

    TEST_SUITE_CLOSURE
};

#endif
