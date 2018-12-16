#ifndef TEST_RBTREE_H
#define TEST_RBTREE_H

TEST(rbtreeInsert);
TEST(rbtreeRemove);
TEST(rbtreeFind);

TEST_SUITE(TSRBTree) {
    TEST_ADD(rbtreeInsert),
    TEST_ADD(rbtreeRemove),
    TEST_ADD(rbtreeFind),
    TEST_SUITE_CLOSURE
};

#endif
