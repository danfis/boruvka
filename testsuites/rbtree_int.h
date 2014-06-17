#ifndef TEST_RBTREE_INT_H
#define TEST_RBTREE_INT_H

TEST(rbtreeIntInsert);
TEST(rbtreeIntRemove);
TEST(rbtreeIntFind);

TEST_SUITE(TSRBTreeInt) {
    TEST_ADD(rbtreeIntInsert),
    TEST_ADD(rbtreeIntRemove),
    TEST_ADD(rbtreeIntFind),
    TEST_SUITE_CLOSURE
};

#endif
