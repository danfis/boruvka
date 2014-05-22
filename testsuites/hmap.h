#ifndef TEST_HMAP_H
#define TEST_HMAP_H

TEST(hmapSetUp);
TEST(hmapTearDown);
TEST(hmapFixed);
TEST(hmapResizable);

TEST_SUITE(TSHMap) {
    TEST_ADD(hmapSetUp),

    TEST_ADD(hmapFixed),
    TEST_ADD(hmapResizable),

    TEST_ADD(hmapTearDown),
    TEST_SUITE_CLOSURE
};

#endif


