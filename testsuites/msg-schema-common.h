#ifndef MSG_SCHEMA_COMMON_H
#define MSG_SCHEMA_COMMON_H

#include <stdio.h>
#include <boruvka/msg-schema.h>
#include "msg-schema.gen.h"

int msgSubEq(const test_submsg_t *a, const test_submsg_t *b);
int msg2ArrEq(const test_msg2_arr_t *a, const test_msg2_arr_t *b);
int msg2Eq(const test_msg2_t *a, const test_msg2_t *b);

void msgSubRand(test_submsg_t *m);
void msg2ArrRand(test_msg2_arr_t *m);
void msg2Rand(test_msg2_t *m);

void msgSubPrint(const test_submsg_t *m, FILE *out);
void msg2ArrPrint(const test_msg2_arr_t *m, FILE *out);
void msg2Print(const test_msg2_t *m, FILE *out);

#endif /* MSG_SCHEMA_COMMON_H */
