#include <unistd.h>
#include <stdio.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"


int main(int argc, char *argv[])
{
    test_msg2_t m1, m2;
    unsigned char *buf;
    int fsize;
    FILE *fin;

    if (argc != 3){
        fprintf(stderr, "Usage: %s in1.bin in2.bin\n", argv[0]);
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

    borMsgDecode(buf, fsize, &m1, test_msg2_schema);
    BOR_FREE(buf);

    fin = fopen(argv[2], "rb");
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

    borMsgDecode(buf, fsize, &m2, test_msg2_schema);
    BOR_FREE(buf);

    if (msg2Eq(&m1, &m2)){
        printf("OK\n");
    }else{
        printf("NOT OK\n");
    }

    return 0;
}

