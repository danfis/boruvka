/***
 * Boruvka
 * --------
 * Copyright (c)2016 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_MSG_SCHEMA_H__
#define __BOR_MSG_SCHEMA_H__

#include <boruvka/core.h>
#include <boruvka/compiler.h>
#include <boruvka/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _BOR_MSG_SCHEMA_INT8         0
#define _BOR_MSG_SCHEMA_UINT8        1
#define _BOR_MSG_SCHEMA_INT16        2
#define _BOR_MSG_SCHEMA_UINT16       3
#define _BOR_MSG_SCHEMA_INT32        4
#define _BOR_MSG_SCHEMA_UINT32       5
#define _BOR_MSG_SCHEMA_INT64        6
#define _BOR_MSG_SCHEMA_UINT64       7
#define _BOR_MSG_SCHEMA_CHAR         8 /*!< Encoded as INT8 */
#define _BOR_MSG_SCHEMA_UCHAR        9 /*!< Encoded as UINT8 */
#define _BOR_MSG_SCHEMA_SHORT       10 /*!< Encoded as INT16 */
#define _BOR_MSG_SCHEMA_USHORT      11 /*!< Encoded as UINT16 */
#define _BOR_MSG_SCHEMA_INT         12 /*!< Encoded as INT32 */
#define _BOR_MSG_SCHEMA_UINT        13 /*!< Encoded as UINT32 */
#define _BOR_MSG_SCHEMA_LONG        14 /*!< Encoded as INT64 */
#define _BOR_MSG_SCHEMA_ULONG       15 /*!< Encoded as UINT64 */
#define _BOR_MSG_SCHEMA_FLOAT       16 /*!< Encoded into INT64 */
#define _BOR_MSG_SCHEMA_DOUBLE      17 /*!< Encoded into INT64 */
#define _BOR_MSG_SCHEMA_MSG         50
#define _BOR_MSG_SCHEMA_ARR_BASE    100

#define _BOR_MSG_SCHEMA_OFFSET(TYPE, MEMBER) bor_offsetof(TYPE, MEMBER)
/*#define _BOR_MSG_SCHEMA_OFFSET(TYPE, MEMBER) ((size_t) &((TYPE * *)0)->MEMBER)*/

typedef struct _bor_msg_schema_t bor_msg_schema_t;

struct _bor_msg_schema_field_t {
    int type;
    int offset;
    int size_offset;
    int alloc_offset;
    const bor_msg_schema_t *schema;
    const void *default_val;
};
typedef struct _bor_msg_schema_field_t bor_msg_schema_field_t;

struct _bor_msg_schema_t {
    int header_offset;
    int struct_bytesize;
    const bor_msg_schema_field_t *field;
    int field_size;
    const void *default_msg;
};

/**
 * Initializes msg to default values.
 */
void borMsgInit(void *msg, const bor_msg_schema_t *schema);

/**
 * Frees allocated memory in arrays.
 */
void borMsgFree(void *msg, const bor_msg_schema_t *schema);

/**
 * Allocates a new initialized msg according to schema.
 */
void *borMsgNew(const bor_msg_schema_t *schema);

/**
 * Deletes msg.
 */
void borMsgDel(void *msg, const bor_msg_schema_t *schema);

/**
 * Set header so it accounts for the fields that differ from the default
 * values.
 * Returns number of changed fields.
 */
int borMsgSetHeader(void *msg, const bor_msg_schema_t *schema);

/**
 * Set the specified field as changed manually using generated HEADER_*
 * macros.
 */
void borMsgSetHeaderField(void *msg, const bor_msg_schema_t *schema,
                          unsigned int field_header_macro);

/**
 * Opposite to borMsgSetHeaderField().
 */
void borMsgUnsetHeaderField(void *msg, const bor_msg_schema_t *schema,
                            unsigned int field_header_macro);

/**
 * Encodes msg according to its schema into buffer *buf.
 * Msg's header must be properly set.
 * Buffer *buf is re-allocated if it needs more memory and it that case
 * *bufsize is changed to the new size.
 * Returns number of bytes used for encoding message or -1 if an error
 * occured.
 */
int borMsgEncode(const void *msg, const bor_msg_schema_t *schema,
                 unsigned char **buf, int *bufsize);

/**
 * Decodes buffer into the message.
 * Returns 0 on success.
 */
int borMsgDecode(const unsigned char *buf, int bufsize,
                 void *msg, const bor_msg_schema_t *schema);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_MSG_SCHEMA_H__ */
