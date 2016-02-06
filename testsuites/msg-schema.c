#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/msg-schema.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#include "msg-schema.struct.h"
#include "msg-schema.schema.c"

TEST(testMsgSchema)
{
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

    assertEquals(borMsgDecodeType(buf, bufsize), 2);

    msg2 = NULL;
    msg2size = 0;
    msgtype = borMsgDecode(buf, bufsize, (void **)&msg2, &msg2size);
    assertEquals(msgtype, 2);
    //assertEquals(memcmp(&msg, msg2, sizeof(test_msg_t)), 0);
}
