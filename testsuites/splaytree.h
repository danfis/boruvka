#ifndef TEST_SPLAYTREE
#define TEST_SPLAYTREE

TEST(splaytreeInsert);
TEST(splaytreeRemove);
TEST(splaytreeFind);

TEST_SUITE(TSSplayTree) {
    TEST_ADD(splaytreeInsert),
    TEST_ADD(splaytreeRemove),
    TEST_ADD(splaytreeFind),
    TEST_SUITE_CLOSURE
};

#endif
