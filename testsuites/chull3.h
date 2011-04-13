#ifndef TEST_CHULL3_H
#define TEST_CHULL3_H

TEST(testCHull);
TEST(testCHullBunny);

TEST_SUITE(TSCHull3){
    TEST_ADD(testCHull),
    TEST_ADD(testCHullBunny),

    TEST_SUITE_CLOSURE
};

#endif
