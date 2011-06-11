#ifndef TEST_PC_H
#define TEST_PC_H

TEST(ppcSetUp);
TEST(ppcTearDown);

TEST(ppcPermutate);
TEST(ppcFromFile);


TEST_SUITE(TSPC) {
    TEST_ADD(ppcSetUp),

    TEST_ADD(ppcPermutate),
    TEST_ADD(ppcFromFile),

    TEST_ADD(ppcTearDown),
    TEST_SUITE_CLOSURE
};

#endif

