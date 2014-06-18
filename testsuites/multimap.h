#ifndef TEST_MULTIMAP_H
#define TEST_MULTIMAP_H

TEST(multimapIntInsert);
TEST(multimapIntRemove);
TEST(multimapIntMin);

TEST_SUITE(TSMultiMap) {
    TEST_ADD(multimapIntInsert),
    TEST_ADD(multimapIntRemove),
    TEST_ADD(multimapIntMin),
    TEST_SUITE_CLOSURE
};

#endif
