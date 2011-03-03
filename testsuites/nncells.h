#ifndef TEST_NNCELLS_H
#define TEST_NNCELLS_H

TEST(nncellsSetUp);
TEST(nncellsTearDown);

TEST(nncellsNew2);

TEST(nncellsEl2);
TEST(nncellsNearest2);
TEST(nncellsNearest6);
/*
TEST(nncellsNearest);
*/

TEST_SUITE(TSNNCells) {
    TEST_ADD(nncellsSetUp),

    TEST_ADD(nncellsNew2),
    TEST_ADD(nncellsEl2),
    TEST_ADD(nncellsNearest2),
    TEST_ADD(nncellsNearest6),
    /*
    TEST_ADD(nncellsNearest),
    */

    TEST_ADD(nncellsTearDown),
    TEST_SUITE_CLOSURE
};

#endif

