#ifndef TEST_VPTREE_HAMMING_H
#define TEST_VPTREE_HAMMING_H


//TEST(vptreeBuild2);
//TEST(vptreeBuild3);
TEST(vptreeHammingAdd);
//TEST(vptreeAddRm);

TEST_SUITE(TSVPTreeHamming) {
    //TEST_ADD(vptreeBuild2),
    //TEST_ADD(vptreeBuild3),

    TEST_ADD(vptreeHammingAdd),
    //TEST_ADD(vptreeAddRm),

    TEST_SUITE_CLOSURE
};

#endif



