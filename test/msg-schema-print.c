#include <unistd.h>
#include <stdio.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"


int main(int argc, char *argv[])
{
    test_msg2_t m;
    unsigned char *buf;
    int fsize;
    FILE *fin;

    if (argc != 2){
        fprintf(stderr, "Usage: %s in.bin\n", argv[0]);
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

    borMsgDecode(buf, fsize, &m, test_msg2_schema);
    msg2Print(&m, stdout);
    BOR_FREE(buf);
    borMsgFree(&m, test_msg2_schema);
    return 0;
}

