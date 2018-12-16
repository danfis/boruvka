#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"


int main(int argc, char *argv[])
{
    test_msg2_t msg;
    unsigned char *buf;
    int fd, bufsize, size, wsize;

    if (argc != 2){
        fprintf(stderr, "Usage: %s out.bin >out.txt\n", argv[0]);
        return -1;
    }

    msg2Rand(&msg);
    borMsgSetHeader(&msg, test_msg2_schema);

    buf = NULL;
    bufsize = 0;
    size = borMsgEncode(&msg, test_msg2_schema, &buf, &bufsize);

    fd = open(argv[1], O_WRONLY | O_CREAT, 0644);
    if (fd < 0){
        fprintf(stderr, "Error: Could not open `%s'\n", argv[1]);
        return -1;
    }

    wsize = 0;
    while (wsize < size)
        wsize += write(fd, buf + wsize, size - wsize);
    close(fd);

    msg2Print(&msg, stdout);
    borMsgFree(&msg, test_msg2_schema);
    BOR_FREE(buf);
    return 0;
}
