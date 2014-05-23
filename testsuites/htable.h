#ifndef TEST_HTABLE_H
#define TEST_HTABLE_H

TEST(htableSetUp);
TEST(htableTearDown);
TEST(htableBasic);
TEST(htableFindAll);
TEST(htableInsertUnique);

TEST_SUITE(TSHTable) {
    TEST_ADD(htableSetUp),

    TEST_ADD(htableBasic),
    TEST_ADD(htableFindAll),
    TEST_ADD(htableInsertUnique),

    TEST_ADD(htableTearDown),
    TEST_SUITE_CLOSURE
};

#endif


