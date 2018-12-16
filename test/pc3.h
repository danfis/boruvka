#ifndef TEST_PC3_H
#define TEST_PC3_H

TEST(pcSetUp);
TEST(pcTearDown);

TEST(pcPermutate);
TEST(pcFromFile);


TEST_SUITE(TSPC3) {
    TEST_ADD(pcSetUp),

    TEST_ADD(pcPermutate),
    TEST_ADD(pcFromFile),

    TEST_ADD(pcTearDown),
    TEST_SUITE_CLOSURE
};

#endif

