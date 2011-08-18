#ifndef TEST_VPTREE_H
#define TEST_VPTREE_H


TEST(vptreeBuild2);
TEST(vptreeBuild3);
TEST(vptreeAdd);
TEST(vptreeAddRm);

TEST_SUITE(TSVPTree) {
    TEST_ADD(vptreeBuild2),
    TEST_ADD(vptreeBuild3),

    TEST_ADD(vptreeAdd),
    TEST_ADD(vptreeAddRm),

    TEST_SUITE_CLOSURE
};

#endif


