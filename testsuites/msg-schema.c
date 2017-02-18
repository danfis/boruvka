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

TEST(testMsgSchemaInit2)
{
    test_msg_t msg;
    test_msg2_t msg2;
    test_submsg_t *submsg;
    int i;

    testMsgInit(&msg);
    msg.dval_size = 10;
    msg.dval = BOR_ALLOC_ARR(double, msg.dval_size);
    testMsgFree(&msg);

    testMsgInit(&msg);
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
    testMsgFree(&msg);

    submsg = testSubmsgNew();
    testSubmsgDel(submsg);

    submsg = testSubmsgNew();
    submsg->arr_size = 12;
    submsg->arr = BOR_ALLOC_ARR(int, 12);
    testSubmsgDel(submsg);

    for (i = 0; i < 10; ++i){
        msg2Rand(&msg2);
        testMsg2Free(&msg2);
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


    borMsgSetHeaderField(&msg, test_msg_schema, TEST_MSG_LVAL);
    assertEquals(msg.__msg_header, 0x1f);
    borMsgUnsetHeaderField(&msg, test_msg_schema, TEST_MSG_DVAL);
    assertEquals(msg.__msg_header, 0x1d);
    borMsgFree(&msg, test_msg_schema);
}

TEST(testMsgSchemaHeader2)
{
    test_msg_t msg;
    test_submsg_t *sub;
    const test_submsg_t *csub;
    double dbls[] = {1., 5., .1, 3., 12. };
    int size;
    const int *cis;

    testMsgInit(&msg);
    assertEquals(testMsgGetIval(&msg), -1);
    assertEquals(testMsgGetDvalSize(&msg), 0);
    assertEquals(testMsgGetDvalAlloc(&msg), 0);
    assertEquals(testMsgGetSubsSize(&msg), 0);
    assertEquals(testMsgGetSubsAlloc(&msg), 0);
    assertEquals(testMsgGetLvalSize(&msg), 0);
    assertEquals(testMsgGetLvalAlloc(&msg), 0);

    testMsgSetIval(&msg, 10);
    assertEquals(testMsgGetIval(&msg), 10);
    assertEquals(testMsgGetDvalSize(&msg), 0);
    assertEquals(testMsgGetSubsSize(&msg), 0);
    assertEquals(testMsgGetLvalSize(&msg), 0);
    assertEquals(msg.__msg_header, 1);

    testMsgSetArrDval(&msg, dbls, 5);
    sub = testMsgSetSub(&msg);
    assertEquals(msg.sub.__msg_header, 0);
    testSubmsgSetI16val(sub, 13);
    assertEquals(msg.sub.__msg_header, 1u << TEST_SUBMSG_I16VAL);
    testSubmsgAddArr(sub, 3);
    testSubmsgAddArr(sub, 4);
    testSubmsgAddArr(sub, -1);
    assertEquals(msg.sub.__msg_header,
                 (1u << TEST_SUBMSG_I16VAL) | (1u << TEST_SUBMSG_ARR));

    assertEquals(testMsgGetIval(&msg), 10);
    assertEquals(testMsgGetDvalSize(&msg), 5);
    assertTrue(testMsgGetDvalAlloc(&msg) >= 5);
    assertEquals(testMsgGetElemDval(&msg, 0), 1.);
    assertEquals(testMsgGetElemDval(&msg, 1), 5.);
    assertEquals(testMsgGetElemDval(&msg, 2), .1);
    assertEquals(testMsgGetElemDval(&msg, 3), 3.);
    assertEquals(testMsgGetElemDval(&msg, 4), 12.);
    csub = testMsgGetSub(&msg);
    cis = testSubmsgGetArr(csub, &size);
    assertEquals(size, 3);
    assertEquals(cis[0], 3);
    assertEquals(cis[1], 4);
    assertEquals(cis[2], -1);
    assertEquals(msg.__msg_header, 7);
    assertEquals(msg.sub.__msg_header,
                 (1u << TEST_SUBMSG_I16VAL) | (1u << TEST_SUBMSG_ARR));
    assertEquals(msg.sub.__msg_header, 0xc);

    testMsgAddDval(&msg, 11.);
    assertEquals(testMsgGetDvalSize(&msg), 6);
    assertTrue(testMsgGetDvalAlloc(&msg) >= 6);


    sub = testMsgAddSubs(&msg);
    testSubmsgSetSval(sub, 10);

    sub = testMsgAddSubs(&msg);
    testSubmsgSetSval(sub, 10);
    testSubmsgSetI16val(sub, 12);

    assertEquals(msg.__msg_header, 0xf);
    assertEquals(msg.sub.__msg_header, 0xc);
    assertEquals(testMsgGetSubsSize(&msg), 2);
    assertEquals(msg.subs[0].__msg_header, 1);
    assertEquals(msg.subs[1].__msg_header, 5);

    sub = testMsgSetElemSubs(&msg, 0);
    testSubmsgSetSval(sub, 0);
    testSubmsgSetHeader(sub);
    assertEquals(msg.subs[0].__msg_header, 0);

    testMsgUnsetSub(&msg);
    assertEquals(msg.__msg_header, 0xb);

    testSubmsgUnsetSval(msg.subs + 0);
    assertEquals(msg.subs[0].__msg_header, 0x0);

    testMsgResizeDval(&msg, 1);
    assertEquals(testMsgGetDvalSize(&msg), 1);

    testMsgReserveDval(&msg, 50);
    assertEquals(testMsgGetDvalSize(&msg), 1);
    assertTrue(testMsgGetDvalAlloc(&msg) >= 50);

    testMsgReserveSubs(&msg, 12);
    assertEquals(testMsgGetSubsSize(&msg), 2);
    assertTrue(testMsgGetSubsAlloc(&msg) >= 12);

    testMsgFree(&msg);
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
        testMsg2Free(&m2);
    }

    if (buf != NULL)
        BOR_FREE(buf);
}
