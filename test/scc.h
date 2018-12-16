#ifndef TEST_SCC_H
#define TEST_SCC_H

TEST(testSCC);

TEST_SUITE(TSSCC) {
    TEST_ADD(testSCC),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_SCC_H */

