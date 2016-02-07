#ifndef TEST_MSG_SCHEMA_H
#define TEST_MSG_SCHEMA_H

TEST(testMsgSchemaInit);
TEST(testMsgSchemaHeader);
TEST(testMsgSchema);

TEST_SUITE(TSMsgSchema) {
    TEST_ADD(testMsgSchemaInit),
    TEST_ADD(testMsgSchemaHeader),
    TEST_ADD(testMsgSchema),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_MSG_SCHEMA_H */
