#ifndef TEST_SPLAYTREE
#define TEST_SPLAYTREE

TEST(splaytreeInsert);
TEST(splaytreeRemove);
TEST(splaytreeFind);
TEST(splaytreeExtractMin);

TEST_SUITE(TSSplayTree) {
    TEST_ADD(splaytreeInsert),
    TEST_ADD(splaytreeRemove),
    TEST_ADD(splaytreeFind),
    TEST_ADD(splaytreeExtractMin),
    TEST_SUITE_CLOSURE
};

#endif
