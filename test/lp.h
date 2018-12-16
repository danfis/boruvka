#ifndef TEST_LP_H
#define TEST_LP_H


TEST(testLP);

TEST_SUITE(TSLP) {
    TEST_ADD(testLP),

    TEST_SUITE_CLOSURE
};

#endif

