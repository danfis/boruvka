
struct test_submsg_t 0x1 {
    sval short; # This is comment
    lval_neco long;
    i16val int16;
}

# This is also comment
struct test_msg_t 2 {
    ival int {-1}; # Default value is -1
    dval double[];
    sub test_submsg_t;
    subs test_submsg_t[];
    lval double[];
}
