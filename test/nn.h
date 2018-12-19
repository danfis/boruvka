#ifndef TEST_NN_H
#define TEST_NN_H


TEST(nnAdd);
TEST(nnAddRm);

TEST_SUITE(TSNN) {
    TEST_ADD(nnAdd),
    TEST_ADD(nnAddRm),

    TEST_SUITE_CLOSURE
};

#endif


