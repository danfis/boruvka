#ifndef TEST_CHULL3_H
#define TEST_CHULL3_H

TEST(testCHull);
TEST(testCHull2);
TEST(testCHull3);
TEST(testCHull4);
TEST(testCHull5);
TEST(testCHull6);
TEST(testCHull7);
TEST(testCHull8);
TEST(testCHullBunny);

TEST_SUITE(TSCHull3){
    TEST_ADD(testCHull),
    TEST_ADD(testCHull2),
    TEST_ADD(testCHull3),
    TEST_ADD(testCHull4),
    TEST_ADD(testCHull5),
    TEST_ADD(testCHull6),
    TEST_ADD(testCHull7),
    TEST_ADD(testCHull8),
    TEST_ADD(testCHullBunny),

    TEST_SUITE_CLOSURE
};

#endif
