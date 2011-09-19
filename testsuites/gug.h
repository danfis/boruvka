#ifndef TEST_GUG_H
#define TEST_GUG_H

TEST(gugSetUp);
TEST(gugTearDown);

TEST(gugNew2);

TEST(gugEl2);
TEST(gugNearest2);
TEST(gugNearest6);
/*
TEST(gugNearest);
*/

TEST_SUITE(TSGUG) {
    TEST_ADD(gugSetUp),

    TEST_ADD(gugNew2),
    TEST_ADD(gugEl2),
    TEST_ADD(gugNearest2),
    TEST_ADD(gugNearest6),
    /*
    TEST_ADD(gugNearest),
    */

    TEST_ADD(gugTearDown),
    TEST_SUITE_CLOSURE
};

#endif

