#include <unistd.h>
#include <stdio.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"
#include "msg-schema.schema.c"


int main(int argc, char *argv[])
{
    test_msg2_t msg;
    unsigned char *buf;
    int fsize, bufsize, size, wsize;
    FILE *fin;

    if (argc != 2){
        fprintf(stderr, "Usage: %s in.bin >out\n", argv[0]);
        return -1;
    }

    fin = fopen(argv[1], "rb");
    if (fin == NULL){
        fprintf(stderr, "Error: Could not open `%s'\n", argv[1]);
        return -1;
    }

    fseek(fin, 0, SEEK_END);
    fsize = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    buf = malloc(fsize + 1);
    fread(buf, fsize, 1, fin);
    fclose(fin);

    borMsgDecode(buf, fsize, &msg, &schema_test_msg2_t);
    BOR_FREE(buf);

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
