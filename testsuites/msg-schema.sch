#ifndef TEST_MSG_SCHEMA_STRUCT
#define TEST_MSG_SCHEMA_STRUCT

msg test_submsg_t {
    sval short; /*!< This is comment */
    lval_neco long;
    i16val int16;
    arr int[];
}

/**
 * This is also comment
 */
msg test_msg_t {
    ival int {-1}; /*!< Default value is -1 */
    dval double[];
    sub test_submsg_t;
    subs test_submsg_t[];
    lval double[];
}

#endif /* TEST_MSG_SCHEMA_STRUCT */
