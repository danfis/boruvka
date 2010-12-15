#ifndef TEST_PC_H
#define TEST_PC_H

TEST(pcSetUp);
TEST(pcTearDown);

TEST(pcPermutate);


TEST_SUITE(TSPC) {
    TEST_ADD(pcSetUp),

    TEST_ADD(pcPermutate),

    TEST_ADD(pcTearDown),
    TEST_SUITE_CLOSURE
};

#endif

