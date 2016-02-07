#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/msg-schema.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "msg-schema.struct.h"
#include "msg-schema.schema.c"

TEST(testMsgSchemaInit)
{
    test_msg_t msg;
    test_submsg_t *submsg;
    int i;

    borMsgInit(&msg, &schema_test_msg_t);
    msg.dval_size = 10;
    msg.dval = BOR_ALLOC_ARR(double, msg.dval_size);
    borMsgFree(&msg, &schema_test_msg_t);

    borMsgInit(&msg, &schema_test_msg_t);
    msg.subs_size = 3;
    msg.subs = BOR_ALLOC_ARR(test_submsg_t, msg.subs_size);
    for (i = 0; i < 3; ++i){
        submsg = msg.subs + i;
        borMsgInit(submsg, &schema_test_submsg_t);
        if (i == 2){
            submsg->arr_size = 4;
            submsg->arr = BOR_ALLOC_ARR(int, 4);
        }
    }
    borMsgFree(&msg, &schema_test_msg_t);

    submsg = borMsgNew(&schema_test_submsg_t);
    borMsgDel(submsg, &schema_test_submsg_t);

    submsg = borMsgNew(&schema_test_submsg_t);
    submsg->arr_size = 12;
    submsg->arr = BOR_ALLOC_ARR(int, 12);
    borMsgDel(submsg, &schema_test_submsg_t);
}

TEST(testMsgSchemaHeader)
{
    test_msg_t msg;

    borMsgInit(&msg, &schema_test_msg_t);
    borMsgSetHeader(&msg, &schema_test_msg_t);
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
    borMsgSetHeader(&msg, &schema_test_msg_t);
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
    borMsgSetHeader(&msg, &schema_test_msg_t);
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
    borMsgInit(msg.subs + 0, &schema_test_submsg_t);
    borMsgInit(msg.subs + 1, &schema_test_submsg_t);
    borMsgSetHeader(&msg, &schema_test_msg_t);
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
    borMsgSetHeader(&msg, &schema_test_msg_t);
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
    borMsgFree(&msg, &schema_test_msg_t);
}

TEST(testMsgSchema)
{
    /*
    test_msg_t msg;
    unsigned char *buf;
    int bufsize, encsize;
    test_msg_t *msg2;
    int msg2size, msgtype;

    memcpy(&msg, schema_test_msg_t.default_msg, sizeof(test_msg_t));
    msg.sub.sval = 10;
    msg.sub.lval_neco = 100938452453L;
    msg.sub.i16val = -123;

    buf = NULL;
    bufsize = 0;
    encsize = borMsgEncode(&msg, &schema_test_msg_t, &buf, &bufsize);

    msg2 = NULL;
    msg2size = 0;
    msgtype = borMsgDecode(buf, bufsize, (void **)&msg2, &msg2size);
    assertEquals(msgtype, 2);
    //assertEquals(memcmp(&msg, msg2, sizeof(test_msg_t)), 0);
    */
}
