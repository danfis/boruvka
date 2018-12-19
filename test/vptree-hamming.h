#ifndef TEST_VPTREE_HAMMING_H
#define TEST_VPTREE_HAMMING_H


TEST(vptreeHammingAdd);
TEST(vptreeHammingAddRm);

TEST_SUITE(TSVPTreeHamming) {
    TEST_ADD(vptreeHammingAdd),
    TEST_ADD(vptreeHammingAddRm),

    TEST_SUITE_CLOSURE
};

#endif



