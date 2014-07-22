#ifndef TEST_LIFO_H
#define TEST_LIFO_H

TEST(lifo1);

TEST_SUITE(TSLifo) {
    TEST_ADD(lifo1),
    TEST_SUITE_CLOSURE
};

#endif

