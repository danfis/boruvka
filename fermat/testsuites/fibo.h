#ifndef TEST_FIBO_H
#define TEST_FIBO_H


TEST(fibo1);

TEST_SUITE(TSFibo) {
    TEST_ADD(fibo1),

    TEST_SUITE_CLOSURE
};

#endif
