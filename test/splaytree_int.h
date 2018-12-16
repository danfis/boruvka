#ifndef TEST_SPLAYTREE_INT
#define TEST_SPLAYTREE_INT

TEST(splaytreeIntInsert);
TEST(splaytreeIntRemove);
TEST(splaytreeIntFind);

TEST_SUITE(TSSplayTreeInt) {
    TEST_ADD(splaytreeIntInsert),
    TEST_ADD(splaytreeIntRemove),
    TEST_ADD(splaytreeIntFind),
    TEST_SUITE_CLOSURE
};

#endif
