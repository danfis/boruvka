#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/msg-schema.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "msg-schema-common.h"

TEST(testMsgSchemaInit)
{
    test_msg_t msg;
    test_msg2_t msg2;
    test_submsg_t *submsg;
    int i;

    borMsgInit(&msg, test_msg_schema);
    msg.dval_size = 10;
    msg.dval = BOR_ALLOC_ARR(double, msg.dval_size);
    borMsgFree(&msg, test_msg_schema);

    borMsgInit(&msg, test_msg_schema);
    msg.subs_size = 3;
    msg.subs = BOR_ALLOC_ARR(test_submsg_t, msg.subs_size);
    for (i = 0; i < 3; ++i){
        submsg = msg.subs + i;
        borMsgInit(submsg, test_submsg_schema);
        if (i == 2){
            submsg->arr_size = 4;
            submsg->arr = BOR_ALLOC_ARR(int, 4);
        }
    }
    borMsgFree(&msg, test_msg_schema);

    submsg = borMsgNew(test_submsg_schema);
    borMsgDel(submsg, test_submsg_schema);

    submsg = borMsgNew(test_submsg_schema);
    submsg->arr_size = 12;
    submsg->arr = BOR_ALLOC_ARR(int, 12);
    borMsgDel(submsg, test_submsg_schema);

    for (i = 0; i < 10; ++i){
        msg2Rand(&msg2);
        borMsgFree(&msg2, test_msg2_schema);
    }
}

TEST(testMsgSchemaHeader)
{
    test_msg_t msg;

    borMsgInit(&msg, test_msg_schema);
    borMsgSetHeader(&msg, test_msg_schema);
    assertEquals(msg.ival, -1);
    assertEquals(msg.dval, NULL);
    assertEquals(msg.dval_size, 0);
    assertEquals(msg.dval_alloc, 0);
    assertEquals(msg.subs, NULL);
    assertEquals(msg.subs_size, 0);
    assertEquals(msg.subs_alloc, 0);
    assertEquals(msg.lval, NULL);
    assertEquals(msg.lval_size, 0);
    assertEquals(msg.lval_alloc, 0);
    assertEquals(msg.sub.__msg_header, 0);
    assertEquals(msg.__msg_header, 0);

    msg.ival = 10;
    borMsgSetHeader(&msg, test_msg_schema);
    assertEquals(msg.ival, 10);
    assertEquals(msg.dval, NULL);
    assertEquals(msg.dval_size, 0);
    assertEquals(msg.dval_alloc, 0);
    assertEquals(msg.subs, NULL);
    assertEquals(msg.subs_size, 0);
    assertEquals(msg.subs_alloc, 0);
    assertEquals(msg.lval, NULL);
    assertEquals(msg.lval_size, 0);
    assertEquals(msg.lval_alloc, 0);
    assertEquals(msg.sub.__msg_header, 0);
    assertEquals(msg.__msg_header, 1);

    msg.dval_size = 5;
    msg.dval = BOR_ALLOC_ARR(double, msg.dval_size);
    msg.sub.i16val = 13;
    msg.sub.arr_size = 3;
    msg.sub.arr = BOR_ALLOC_ARR(int, msg.sub.arr_size);
    borMsgSetHeader(&msg, test_msg_schema);
    assertEquals(msg.ival, 10);
    assertNotEquals(msg.dval, NULL);
    assertEquals(msg.dval_size, 5);
    assertEquals(msg.dval_alloc, 0);
    assertEquals(msg.subs, NULL);
    assertEquals(msg.subs_size, 0);
    assertEquals(msg.subs_alloc, 0);
    assertEquals(msg.lval, NULL);
    assertEquals(msg.lval_size, 0);
    assertEquals(msg.lval_alloc, 0);
    assertEquals(msg.sub.__msg_header, 0xc);
    assertEquals(msg.__msg_header, 7);


    msg.subs_size = 2;
    msg.subs = BOR_ALLOC_ARR(test_submsg_t, msg.subs_size);
    borMsgInit(msg.subs + 0, test_submsg_schema);
    borMsgInit(msg.subs + 1, test_submsg_schema);
    borMsgSetHeader(&msg, test_msg_schema);
    assertEquals(msg.ival, 10);
    assertNotEquals(msg.dval, NULL);
    assertEquals(msg.dval_size, 5);
    assertEquals(msg.dval_alloc, 0);
    assertNotEquals(msg.subs, NULL);
    assertEquals(msg.subs_size, 2);
    assertEquals(msg.subs_alloc, 0);
    assertEquals(msg.lval, NULL);
    assertEquals(msg.lval_size, 0);
    assertEquals(msg.lval_alloc, 0);
    assertEquals(msg.sub.__msg_header, 0xc);
    assertEquals(msg.__msg_header, 0xf);
    assertEquals(msg.subs[0].__msg_header, 0);
    assertEquals(msg.subs[1].__msg_header, 0);


    msg.subs[0].sval = 10;
    msg.subs[1].sval = 10;
    msg.subs[1].i16val = 12;
    borMsgSetHeader(&msg, test_msg_schema);
    assertEquals(msg.ival, 10);
    assertNotEquals(msg.dval, NULL);
    assertEquals(msg.dval_size, 5);
    assertEquals(msg.dval_alloc, 0);
    assertNotEquals(msg.subs, NULL);
    assertEquals(msg.subs_size, 2);
    assertEquals(msg.subs_alloc, 0);
    assertEquals(msg.lval, NULL);
    assertEquals(msg.lval_size, 0);
    assertEquals(msg.lval_alloc, 0);
    assertEquals(msg.sub.__msg_header, 0xc);
    assertEquals(msg.__msg_header, 0xf);
    assertEquals(msg.subs[0].__msg_header, 1);
    assertEquals(msg.subs[1].__msg_header, 0x5);


    borMsgSetHeaderField(&msg, test_msg_schema, BOR_MSG_HEADER_test_msg_lval);
    assertEquals(msg.__msg_header, 0x1f);
    borMsgUnsetHeaderField(&msg, test_msg_schema, BOR_MSG_HEADER_test_msg_dval);
    assertEquals(msg.__msg_header, 0x1d);
    borMsgFree(&msg, test_msg_schema);
}

TEST(testMsgSchema)
{
    test_msg2_t m1, m2;
    unsigned char *buf;
    int i, bufsize, size;

    buf = NULL;
    bufsize = 0;
    for (i = 0; i < 100; ++i){
        msg2Rand(&m1);
        borMsgSetHeader(&m1, test_msg2_schema);

        size = borMsgEncode(&m1, test_msg2_schema, &buf, &bufsize);

        borMsgDecode(buf, size, &m2, test_msg2_schema);
        assertTrue(msg2Eq(&m1, &m2));

        borMsgFree(&m1, test_msg2_schema);
        borMsgFree(&m2, test_msg2_schema);
    }

    if (buf != NULL)
        BOR_FREE(buf);
}
