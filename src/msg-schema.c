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

#include <endian.h>
#include <stdio.h>

#include "boruvka/msg-schema.h"
#include "boruvka/alloc.h"

#define HEADER_TYPE uint32_t
#define HEADER_TO_LE htole32
#define HEADER_TO_H le32toh
#define ARR_LEN_TYPE uint32_t
#define ARR_LEN_TYPE_SIZE sizeof(ARR_LEN_TYPE)
#define ARR_LEN_TO_LE htole32
#define ARR_LEN_TO_H le32toh

#define FIELD_PTR(msg_struct, offset) \
    (void *)(((unsigned char *)(msg_struct)) + (offset))
#define FIELD(msg_struct, offset, type) \
    (*(type *)FIELD_PTR((msg_struct), (offset)))

#define SET_HEADER(header, idx) \
    (header) |= (1u << (idx))

#ifdef BOR_LITTLE_ENDIAN
# define SET_ENDIAN(header) (header) |= (0x1u << 31u)
# define CHECK_ENDIAN(header) (((header) & (0x1u << 31u)) == (0x1u << 31u))
# define CONV_END_int32_t be32toh
# define CONV_END_int64_t be64toh
# define CONV_HEADER(header)
# define CONV_ARR_LEN(len)
#endif

#ifdef BOR_BIG_ENDIAN
# define SET_ENDIAN(header) (header) &= ~(0x1u << 31u)
# define CHECK_ENDIAN(header) (((header) & (0x1u << 31u)) == 0u)
# define CONV_END_int32_t le32toh
# define CONV_END_int64_t le64toh
# define CONV_HEADER(header) ((header) = HEADER_TO_LE(header))
# define CONV_ARR_LEN(len) ((len) = ARR_LEN_TO_LE(len))
#endif

#define CONV_ENDIAN(msg, offset, type) \
    FIELD((msg), (offset), type) = CONV_END_##type(FIELD((msg), (offset), type))

#if !defined(BOR_LITTLE_ENDIAN) && !defined(BOR_BIG_ENDIAN)
# error "Cannot determie endianness!!"
#endif

#define MAX_TYPE_ID 18

static int type_size[MAX_TYPE_ID] = {
    1, /*_BOR_MSG_SCHEMA_INT8 */
    1, /*_BOR_MSG_SCHEMA_UINT8 */
    2, /*_BOR_MSG_SCHEMA_INT16 */
    2, /*_BOR_MSG_SCHEMA_UINT16 */
    4, /*_BOR_MSG_SCHEMA_INT32 */
    4, /*_BOR_MSG_SCHEMA_UINT32 */
    8, /*_BOR_MSG_SCHEMA_INT64 */
    8, /*_BOR_MSG_SCHEMA_UINT64 */
    1, /*_BOR_MSG_SCHEMA_CHAR */
    1, /*_BOR_MSG_SCHEMA_UCHAR */
    2, /*_BOR_MSG_SCHEMA_SHORT */
    2, /*_BOR_MSG_SCHEMA_USHORT */
    4, /*_BOR_MSG_SCHEMA_INT */
    4, /*_BOR_MSG_SCHEMA_UINT */
    8, /*_BOR_MSG_SCHEMA_LONG */
    8, /*_BOR_MSG_SCHEMA_ULONG */
    8, /*_BOR_MSG_SCHEMA_FLOAT */
    8 /*_BOR_MSG_SCHEMA_DOUBLE */
};

struct _wbuf_t {
    unsigned char *buf;
    int size;
    int w;
};
typedef struct _wbuf_t wbuf_t;


static void encode(wbuf_t *wbuf, const void *msg,
                   const bor_msg_schema_t *_schema);

_bor_inline int cmpFieldDefault(const void *msg,
                                const bor_msg_schema_field_t *field)
{
    return memcmp(field->default_val, FIELD_PTR(msg, field->offset),
                  type_size[field->type]);
}

_bor_inline void W(wbuf_t *wbuf, void *data, int size)
{
    if (wbuf->w + size > wbuf->size){
        wbuf->size *= 2;
        if (wbuf->size - wbuf->w < size)
            wbuf->size += size;
        wbuf->buf = BOR_REALLOC_ARR(wbuf->buf, unsigned char, wbuf->size);
    }
    memcpy(wbuf->buf + wbuf->w, data, size);
    wbuf->w += size;
}

_bor_inline void wHeader(wbuf_t *wbuf, HEADER_TYPE header)
{
    SET_ENDIAN(header);
    CONV_HEADER(header);
    W(wbuf, &header, sizeof(HEADER_TYPE));
}

_bor_inline void wArrLen(wbuf_t *wbuf, int _len)
{
    ARR_LEN_TYPE len = _len;
    CONV_ARR_LEN(len);
    W(wbuf, &len, ARR_LEN_TYPE_SIZE);
}

_bor_inline void wField(wbuf_t *wbuf, const void *msg, int offset, int size)
{
    void *v = FIELD_PTR(msg, offset);
    W(wbuf, v, size);
}

_bor_inline void wArr(wbuf_t *wbuf, const void *msg, int offset,
                      int len, int size)
{
    wArrLen(wbuf, len);
    W(wbuf, FIELD(msg, offset, void *), size * len);
}

_bor_inline void wMsgArr(wbuf_t *wbuf, const void *msg, int offset,
                         int len, const bor_msg_schema_t *sub_schema)
{
    int i, size;
    void *submsg;

    size = sub_schema->struct_bytesize;
    wArrLen(wbuf, len);

    submsg = FIELD(msg, offset, void *);
    for (i = 0; i < len; ++i){
        encode(wbuf, submsg, sub_schema);
        submsg = ((char *)submsg) + size;
    }
}

_bor_inline uint32_t rHeader(unsigned char **rbuf)
{
    uint32_t header;
    header = *(uint32_t *)*rbuf;
#ifdef BOR_BIG_ENDIAN
    header = le32toh(header);
#endif
    *rbuf += sizeof(header);
    return header;
}

_bor_inline int rArrLen(unsigned char **rbuf)
{
    int len;
#ifdef BOR_BIG_ENDIAN
    len = ARR_LEN_TO_H(*(ARR_LEN_TYPE *)*rbuf);
#else
    len = *(ARR_LEN_TYPE *)*rbuf;
#endif
    *rbuf += ARR_LEN_TYPE_SIZE;
    return len;
}

static void encode(wbuf_t *wbuf, const void *msg,
                   const bor_msg_schema_t *schema)
{
    const bor_msg_schema_field_t *field;
    const void *sub_msg;
    HEADER_TYPE header = FIELD(msg, schema->header_offset, HEADER_TYPE);
    int i, type, len;

    wHeader(wbuf, header);
    for (i = 0; i < schema->field_size; ++i){
        if (header & 0x1u){
            field = schema->field + i;

            if (field->type < MAX_TYPE_ID){
                wField(wbuf, msg, field->offset, type_size[field->type]);

            }else if (field->type == _BOR_MSG_SCHEMA_MSG){
                sub_msg = FIELD_PTR(msg, field->offset);
                encode(wbuf, sub_msg, field->schema);

            }else if (field->type >= _BOR_MSG_SCHEMA_ARR_BASE){
                type = field->type - _BOR_MSG_SCHEMA_ARR_BASE;
                len = FIELD(msg, field->size_offset, int);

                if (type < MAX_TYPE_ID){
                    wArr(wbuf, msg, field->offset, len, type_size[type]);

                }else{
                    wMsgArr(wbuf, msg, field->offset, len, field->schema);
                }
            }
        }

        header >>= 1u;
    }
}

static void msgArrSetHeader(void *msg, int offset, int len,
                            const bor_msg_schema_t *sub_schema)
{
    int i, size;
    void *submsg;

    size = sub_schema->struct_bytesize;

    submsg = FIELD(msg, offset, void *);
    for (i = 0; i < len; ++i){
        borMsgSetHeader(submsg, sub_schema);
        submsg = ((char *)submsg) + size;
    }
}

static void msgArrFree(void *msg, int offset, int len,
                       const bor_msg_schema_t *sub_schema)
{
    int i, size;
    void *submsg;

    size = sub_schema->struct_bytesize;

    submsg = FIELD(msg, offset, void *);
    for (i = 0; i < len; ++i){
        borMsgFree(submsg, sub_schema);
        submsg = ((char *)submsg) + size;
    }
}

void borMsgInit(void *msg, const bor_msg_schema_t *schema)
{
    memcpy(msg, schema->default_msg, schema->struct_bytesize);
}

void borMsgFree(void *msg, const bor_msg_schema_t *schema)
{
    const bor_msg_schema_field_t *field;
    int i, type, len;

    for (i = 0; i < schema->field_size; ++i){
        field = schema->field + i;

        if (field->type == _BOR_MSG_SCHEMA_MSG){
            borMsgFree(FIELD_PTR(msg, field->offset), field->schema);

        }else if (field->type >= _BOR_MSG_SCHEMA_ARR_BASE){
            type = field->type - _BOR_MSG_SCHEMA_ARR_BASE;
            len = FIELD(msg, field->size_offset, int);

            if (len <= 0)
                continue;

            if (type == _BOR_MSG_SCHEMA_MSG)
                msgArrFree(msg, field->offset, len, field->schema);
            BOR_FREE(FIELD(msg, field->offset, void *));
        }
    }

    FIELD(msg, schema->header_offset, HEADER_TYPE) = 0;
}

void *borMsgNew(const bor_msg_schema_t *schema)
{
    void *msg;

    msg = (void *)BOR_ALLOC_ARR(char, schema->struct_bytesize);
    borMsgInit(msg, schema);
    return msg;
}

void borMsgDel(void *msg, const bor_msg_schema_t *schema)
{
    borMsgFree(msg, schema);
    BOR_FREE(msg);
}

int borMsgSetHeader(void *msg, const bor_msg_schema_t *schema)
{
    const bor_msg_schema_field_t *field;
    void *sub_msg;
    HEADER_TYPE header = 0u;
    int i, type, len, change = 0;

    for (i = 0; i < schema->field_size; ++i){
        field = schema->field + i;

        if (field->type < MAX_TYPE_ID){
            if (cmpFieldDefault(msg, field) != 0){
                SET_HEADER(header, i);
                change += 1;
            }

        }else if (field->type == _BOR_MSG_SCHEMA_MSG){
            sub_msg = FIELD_PTR(msg, field->offset);
            if (borMsgSetHeader(sub_msg, field->schema) > 0){
                SET_HEADER(header, i);
                change += 1;
            }

        }else if (field->type >= _BOR_MSG_SCHEMA_ARR_BASE){
            type = field->type - _BOR_MSG_SCHEMA_ARR_BASE;
            len = FIELD(msg, field->size_offset, int);

            if (len <= 0)
                continue;

            SET_HEADER(header, i);
            change += 1;
            if (type >= MAX_TYPE_ID)
                msgArrSetHeader(msg, field->offset, len, field->schema);
        }
    }

    FIELD(msg, schema->header_offset, HEADER_TYPE) = header;
    return change;
}

void borMsgSetHeaderField(void *msg, const bor_msg_schema_t *schema,
                          unsigned int idx)
{
    FIELD(msg, schema->header_offset, HEADER_TYPE) |= (1u << idx);
}

void borMsgUnsetHeaderField(void *msg, const bor_msg_schema_t *schema,
                            unsigned int idx)
{
    FIELD(msg, schema->header_offset, HEADER_TYPE) &= ~(1u << idx);
}

int borMsgEncode(const void *msg, const bor_msg_schema_t *schema,
                 unsigned char **buf, int *bufsize)
{
    wbuf_t wbuf = { *buf, *bufsize, 0 };

    encode(&wbuf, msg, schema);

    *buf = wbuf.buf;
    *bufsize = wbuf.size;
    return wbuf.w;
}

int borMsgDecode(const unsigned char *buf, int bufsize,
                 void **msg, int *msgsize)
{
    return -1;
}
