#include <unistd.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"
#include "msg-schema.schema.c"


int main(int argc, char *argv[])
{
    test_msg2_t msg;
    unsigned char *buf;
    int bufsize, size, wsize;

    msg2Rand(&msg);
    borMsgSetHeader(&msg, &schema_test_msg2_t);

    buf = NULL;
    bufsize = 0;
    size = borMsgEncode(&msg, &schema_test_msg2_t, &buf, &bufsize);

    wsize = 0;
    while (wsize < size)
        wsize += write(1, buf + wsize, size - wsize);

    borMsgFree(&msg, &schema_test_msg2_t);
    BOR_FREE(buf);
    return 0;
}
