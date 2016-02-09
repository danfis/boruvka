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
# define CONV_HEADER(header)
# define UNCONV_HEADER(header)
# define CONV_ARR_LEN(len)
# define UNCONV_ARR_LEN(len)

# define TO_LE_int8_t(x) x
# define TO_LE_uint8_t(x) x
# define TO_LE_int16_t(x) x
# define TO_LE_uint16_t(x) x
# define TO_LE_int32_t(x) x
# define TO_LE_uint32_t(x) x
# define TO_LE_int64_t(x) x
# define TO_LE_uint64_t(x) x

# define TO_H_int8_t(x) x
# define TO_H_uint8_t(x) x
# define TO_H_int16_t(x) x
# define TO_H_uint16_t(x) x
# define TO_H_int32_t(x) x
# define TO_H_uint32_t(x) x
# define TO_H_int64_t(x) x
# define TO_H_uint64_t(x) x
#endif

#ifdef BOR_BIG_ENDIAN
# define CONV_HEADER(header) ((header) = HEADER_TO_LE(header))
# define UNCONV_HEADER(header) ((header) = HEADER_TO_H(header))
# define CONV_ARR_LEN(len) ((len) = ARR_LEN_TO_LE(len))
# define UNCONV_ARR_LEN(len) ((len) = ARR_LEN_TO_H(len))

# define TO_LE_int8_t(x) x
# define TO_LE_uint8_t(x) TO_LE_int8_t(x)
# define TO_LE_int16_t(x) htole16(x)
# define TO_LE_uint16_t(x) TO_LE_int16_t(x)
# define TO_LE_int32_t(x) htole32(x)
# define TO_LE_uint32_t(x) TO_LE_int32_t(x)
# define TO_LE_int64_t(x) htole64(x)
# define TO_LE_uint64_t(x) TO_LE_int64_t(x)

# define TO_H_int8_t(x) x
# define TO_H_uint8_t(x) TO_H_int8_t(x)
# define TO_H_int16_t(x) le16toh(x)
# define TO_H_uint16_t(x) TO_H_int16_t(x)
# define TO_H_int32_t(x) le32toh(x)
# define TO_H_uint32_t(x) TO_H_int32_t(x)
# define TO_H_int64_t(x) le64toh(x)
# define TO_H_uint64_t(x) TO_H_int64_t(x)
#endif

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))
static uint64_t pack754(long double f, unsigned bits, unsigned expbits);
static long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

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
static void decode(unsigned char **rbuf, void *msg,
                   const bor_msg_schema_t *schema);

_bor_inline int cmpFieldDefault(const void *msg,
                                const bor_msg_schema_field_t *field)
{
    return memcmp(field->default_val, FIELD_PTR(msg, field->offset),
                  type_size[field->type]);
}

_bor_inline void W(wbuf_t *wbuf, const void *data, int size)
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
    CONV_HEADER(header);
    W(wbuf, &header, sizeof(HEADER_TYPE));
}

_bor_inline HEADER_TYPE rHeader(unsigned char **rbuf)
{
    HEADER_TYPE header;
    header = *(HEADER_TYPE *)*rbuf;
    UNCONV_HEADER(header);
    *rbuf += sizeof(header);
    return header;
}

_bor_inline void wArrLen(wbuf_t *wbuf, int _len)
{
    ARR_LEN_TYPE len = _len;
    CONV_ARR_LEN(len);
    W(wbuf, &len, ARR_LEN_TYPE_SIZE);
}

_bor_inline int rArrLen(unsigned char **rbuf)
{
    int len = *(ARR_LEN_TYPE *)*rbuf;
    UNCONV_ARR_LEN(len);
    *rbuf += ARR_LEN_TYPE_SIZE;
    return len;
}

#define W_FIELD(wbuf, data, from_type, to_type) \
    do { \
        to_type v = *(from_type *)data; \
        v = TO_LE_##to_type(v); \
        W(wbuf, &v, sizeof(v)); \
    } while (0)

#define W_FIELD_FLT(wbuf, data, from_type) \
    do { \
        from_type f = *(from_type *)data; \
        uint64_t v = pack754_64(f); \
        v = TO_LE_uint64_t(v); \
        W(wbuf, &v, sizeof(v)); \
    } while (0)

void wField(wbuf_t *wbuf, const void *msg, int offset, int type)
{
    const void *data = FIELD_PTR(msg, offset);

    switch (type){
#ifdef BOR_BIG_ENDIAN
        case _BOR_MSG_SCHEMA_INT16:
            W_FIELD(wbuf, data, int16_t, int16_t);
            break;
        case _BOR_MSG_SCHEMA_UINT16:
            W_FIELD(wbuf, data, uint16_t, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT32:
            W_FIELD(wbuf, data, int32_t, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT32:
            W_FIELD(wbuf, data, uint32_t, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_INT64:
            W_FIELD(wbuf, data, int64_t, int64_t);
            break;
        case _BOR_MSG_SCHEMA_UINT64:
            W_FIELD(wbuf, data, uint64_t, uint64_t);
            break;
#endif /* BOR_BIG_ENDIAN */
        case _BOR_MSG_SCHEMA_CHAR:
            W_FIELD(wbuf, data, char, int8_t);
            break;
        case _BOR_MSG_SCHEMA_UCHAR:
            W_FIELD(wbuf, data, unsigned char, uint8_t);
            break;
        case _BOR_MSG_SCHEMA_SHORT:
            W_FIELD(wbuf, data, short, int16_t);
            break;
        case _BOR_MSG_SCHEMA_USHORT:
            W_FIELD(wbuf, data, unsigned short, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT:
            W_FIELD(wbuf, data, int, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT:
            W_FIELD(wbuf, data, unsigned int, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_LONG:
            W_FIELD(wbuf, data, long, int64_t);
            break;
        case _BOR_MSG_SCHEMA_ULONG:
            W_FIELD(wbuf, data, unsigned long, uint64_t);
            break;
        case _BOR_MSG_SCHEMA_FLOAT:
            W_FIELD_FLT(wbuf, data, float);
            break;
        case _BOR_MSG_SCHEMA_DOUBLE:
            W_FIELD_FLT(wbuf, data, double);
            break;
        default:
            W(wbuf, data, type_size[type]);
    }
}

#define R_FIELD(rbuf, dst, to_type, from_type) \
    *((to_type *)dst) = TO_H_##from_type(*((from_type *)*rbuf))

#define R_FIELD_FLT(rbuf, dst, to_type) \
    do { \
        uint64_t v = *((uint64_t *)*rbuf); \
        v = TO_H_uint64_t(v); \
        *((to_type *)dst) = unpack754_64(v); \
    } while (0)

_bor_inline void rField(unsigned char **rbuf, void *msg, int off, int type)
{
    int size = type_size[type];
    void *dst = FIELD_PTR(msg, off);

    switch (type){
#ifdef BOR_BIG_ENDIAN
        case _BOR_MSG_SCHEMA_INT16:
            R_FIELD(rbuf, dst, int16_t, int16_t);
            break;
        case _BOR_MSG_SCHEMA_UINT16:
            R_FIELD(rbuf, dst, uint16_t, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT32:
            R_FIELD(rbuf, dst, int32_t, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT32:
            R_FIELD(rbuf, dst, uint32_t, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_INT64:
            R_FIELD(rbuf, dst, int64_t, int64_t);
            break;
        case _BOR_MSG_SCHEMA_UINT64:
            R_FIELD(rbuf, dst, uint64_t, uint64_t);
            break;
#endif /* BOR_BIG_ENDIAN */
        case _BOR_MSG_SCHEMA_CHAR:
            R_FIELD(rbuf, dst, char, int8_t);
            break;
        case _BOR_MSG_SCHEMA_UCHAR:
            R_FIELD(rbuf, dst, unsigned char, uint8_t);
            break;
        case _BOR_MSG_SCHEMA_SHORT:
            R_FIELD(rbuf, dst, short, int16_t);
            break;
        case _BOR_MSG_SCHEMA_USHORT:
            R_FIELD(rbuf, dst, unsigned short, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT:
            R_FIELD(rbuf, dst, int, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT:
            R_FIELD(rbuf, dst, unsigned int, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_LONG:
            R_FIELD(rbuf, dst, long, int64_t);
            break;
        case _BOR_MSG_SCHEMA_ULONG:
            R_FIELD(rbuf, dst, unsigned long, uint64_t);
            break;
        case _BOR_MSG_SCHEMA_FLOAT:
            R_FIELD_FLT(rbuf, dst, float);
            break;
        case _BOR_MSG_SCHEMA_DOUBLE:
            R_FIELD_FLT(rbuf, dst, double);
            break;
        default:
            memcpy(dst, *rbuf, size);
    }
    *rbuf += size;
}


#ifdef BOR_LITTLE_ENDIAN
# define ARR_FAST_COND(from, to) sizeof(from) == sizeof(to)
#else /* BOR_LITTLE_ENDIAN */
# define ARR_FAST_COND(from, to) (sizeof(from) == sizeof(to)) && sizeof(from) == 1
#endif /* BOR_LITTLE_ENDIAN */

#define W_ARR(wbuf, arr, len, from_type, to_type) \
    do { \
        if (ARR_FAST_COND(from_type, to_type)){ \
            W(wbuf, arr, sizeof(to_type) * len); \
        }else{ \
            to_type v; \
            from_type *arr2 = (from_type *)arr; \
            int i; \
            \
            for (i = 0; i < (len); ++i){ \
                v = arr2[i]; \
                v = TO_LE_##to_type(v); \
                W(wbuf, &v, sizeof(v)); \
            } \
        } \
    } while (0)

#define W_ARR_FLT(wbuf, arr, len, from_type) \
    do { \
        uint64_t v; \
        from_type *arr2 = (from_type *)arr; \
        int i; \
        \
        for (i = 0; i < (len); ++i){ \
            v = pack754_64(arr2[i]); \
            v = TO_LE_uint64_t(v); \
            W(wbuf, &v, sizeof(v)); \
        } \
    } while (0)

_bor_inline void wArr(wbuf_t *wbuf, const void *msg, int offset,
                      int len, int type)
{
    const void *arr = FIELD(msg, offset, void *);

    wArrLen(wbuf, len);
    switch (type){
#ifdef BOR_BIG_ENDIAN
        case _BOR_MSG_SCHEMA_INT16:
            W_ARR(wbuf, arr, len, int16_t, int16_t);
            break;
        case _BOR_MSG_SCHEMA_UINT16:
            W_ARR(wbuf, arr, len, uint16_t, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT32:
            W_ARR(wbuf, arr, len, int32_t, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT32:
            W_ARR(wbuf, arr, len, uint32_t, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_INT64:
            W_ARR(wbuf, arr, len, int64_t, int64_t);
            break;
        case _BOR_MSG_SCHEMA_UINT64:
            W_ARR(wbuf, arr, len, uint64_t, uint64_t);
            break;
#endif /* BOR_BIG_ENDIAN */
        case _BOR_MSG_SCHEMA_CHAR:
            W_ARR(wbuf, arr, len, char, int8_t);
            break;
        case _BOR_MSG_SCHEMA_UCHAR:
            W_ARR(wbuf, arr, len, unsigned char, uint8_t);
            break;
        case _BOR_MSG_SCHEMA_SHORT:
            W_ARR(wbuf, arr, len, short, int16_t);
            break;
        case _BOR_MSG_SCHEMA_USHORT:
            W_ARR(wbuf, arr, len, unsigned short, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT:
            W_ARR(wbuf, arr, len, int, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT:
            W_ARR(wbuf, arr, len, unsigned int, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_LONG:
            W_ARR(wbuf, arr, len, long, int64_t);
            break;
        case _BOR_MSG_SCHEMA_ULONG:
            W_ARR(wbuf, arr, len, unsigned long, uint64_t);
            break;
        case _BOR_MSG_SCHEMA_FLOAT:
            W_ARR_FLT(wbuf, arr, len, float);
            break;
        case _BOR_MSG_SCHEMA_DOUBLE:
            W_ARR_FLT(wbuf, arr, len, double);
            break;
        default:
            W(wbuf, FIELD(msg, offset, void *), type_size[type] * len);
    }
}

#define R_ARR(rbuf, msg, offset, len, to_type, from_type) \
    do { \
        to_type *buf = BOR_ALLOC_ARR(to_type, len); \
        if (ARR_FAST_COND(from_type, to_type)){ \
            memcpy(buf, *rbuf, sizeof(to_type) * len); \
            FIELD(msg, offset, to_type *) = buf; \
        }else{ \
            from_type *arr = (from_type *)*rbuf; \
            int i; \
            \
            for (i = 0; i < (len); ++i){ \
                buf[i] = TO_H_##from_type(arr[i]); \
            } \
            FIELD(msg, offset, to_type *) = buf; \
        } \
    } while (0)

#define R_ARR_FLT(rbuf, msg, offset, len, to_type) \
    do { \
        to_type *buf = BOR_ALLOC_ARR(to_type, len); \
        uint64_t v, *arr = (uint64_t *)*rbuf; \
        int i; \
        \
        for (i = 0; i < (len); ++i){ \
            v = arr[i]; \
            v = TO_H_uint64_t(v); \
            buf[i] = unpack754_64(v); \
        } \
        FIELD(msg, offset, to_type *) = buf; \
    } while (0)

_bor_inline void rArr(unsigned char **rbuf, void *msg, int offset, int len,
                      int size_off, int alloc_off, int type)
{
    int size = type_size[type];
    switch (type){
#ifdef BOR_BIG_ENDIAN
        case _BOR_MSG_SCHEMA_INT16:
            R_ARR(rbuf, msg, offset, len, int16_t, int16_t);
            break;
        case _BOR_MSG_SCHEMA_UINT16:
            R_ARR(rbuf, msg, offset, len, uint16_t, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT32:
            R_ARR(rbuf, msg, offset, len, int32_t, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT32:
            R_ARR(rbuf, msg, offset, len, uint32_t, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_INT64:
            R_ARR(rbuf, msg, offset, len, int64_t, int64_t);
            break;
        case _BOR_MSG_SCHEMA_UINT64:
            R_ARR(rbuf, msg, offset, len, uint64_t, uint64_t);
            break;
#endif /* BOR_BIG_ENDIAN */
        case _BOR_MSG_SCHEMA_CHAR:
            R_ARR(rbuf, msg, offset, len, char, int8_t);
            break;
        case _BOR_MSG_SCHEMA_UCHAR:
            R_ARR(rbuf, msg, offset, len, unsigned char, uint8_t);
            break;
        case _BOR_MSG_SCHEMA_SHORT:
            R_ARR(rbuf, msg, offset, len, short, int16_t);
            break;
        case _BOR_MSG_SCHEMA_USHORT:
            R_ARR(rbuf, msg, offset, len, unsigned short, uint16_t);
            break;
        case _BOR_MSG_SCHEMA_INT:
            R_ARR(rbuf, msg, offset, len, int, int32_t);
            break;
        case _BOR_MSG_SCHEMA_UINT:
            R_ARR(rbuf, msg, offset, len, unsigned int, uint32_t);
            break;
        case _BOR_MSG_SCHEMA_LONG:
            R_ARR(rbuf, msg, offset, len, long, int64_t);
            break;
        case _BOR_MSG_SCHEMA_ULONG:
            R_ARR(rbuf, msg, offset, len, unsigned long, uint64_t);
            break;
        case _BOR_MSG_SCHEMA_FLOAT:
            R_ARR_FLT(rbuf, msg, offset, len, float);
            break;
        case _BOR_MSG_SCHEMA_DOUBLE:
            R_ARR_FLT(rbuf, msg, offset, len, double);
            break;
        default:
            {
                void *buf = BOR_ALLOC_ARR(char, size * len);
                memcpy(buf, *rbuf, size * len);
                FIELD(msg, offset, void *) = buf;
            }
    }

    FIELD(msg, size_off, int) = len;
    FIELD(msg, alloc_off, int) = len;
    *rbuf += size * len;
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

_bor_inline void rMsgArr(unsigned char **rbuf, void *msg, int offset, int len,
                         int size_off, int alloc_off,
                         const bor_msg_schema_t *schema)
{
    int i, size;
    void *buf, *wbuf;

    size = schema->struct_bytesize;
    buf = BOR_ALLOC_ARR(char, size * len);

    wbuf = buf;
    for (i = 0; i < len; ++i){
        borMsgInit(wbuf, schema);
        decode(rbuf, wbuf, schema);
        wbuf = (((char *)wbuf) + size);
    }

    FIELD(msg, offset, void *) = buf;
    FIELD(msg, size_off, int) = len;
    FIELD(msg, alloc_off, int) = len;
}

static uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

static long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
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
                wField(wbuf, msg, field->offset, field->type);

            }else if (field->type == _BOR_MSG_SCHEMA_MSG){
                sub_msg = FIELD_PTR(msg, field->offset);
                encode(wbuf, sub_msg, field->schema);

            }else if (field->type >= _BOR_MSG_SCHEMA_ARR_BASE){
                type = field->type - _BOR_MSG_SCHEMA_ARR_BASE;
                len = FIELD(msg, field->size_offset, int);

                if (type < MAX_TYPE_ID){
                    wArr(wbuf, msg, field->offset, len, type);

                }else{
                    wMsgArr(wbuf, msg, field->offset, len, field->schema);
                }
            }
        }

        header >>= 1u;
    }
}

static void decode(unsigned char **rbuf, void *msg,
                   const bor_msg_schema_t *schema)
{
    const bor_msg_schema_field_t *field;
    uint32_t header;
    void *sub_msg;
    int type, i, len;

    header = rHeader(rbuf);
    FIELD(msg, schema->header_offset, HEADER_TYPE) = header;

    for (i = 0; i < schema->field_size; ++i){
        if (header & 0x1u){
            field = schema->field + i;

            if (field->type < MAX_TYPE_ID){
                rField(rbuf, msg, field->offset, field->type);

            }else if (field->type == _BOR_MSG_SCHEMA_MSG){
                sub_msg = FIELD_PTR(msg, field->offset);
                decode(rbuf, sub_msg, field->schema);

            }else if (field->type >= _BOR_MSG_SCHEMA_ARR_BASE){
                type = field->type - _BOR_MSG_SCHEMA_ARR_BASE;
                len = rArrLen(rbuf);

                if (type < MAX_TYPE_ID){
                    rArr(rbuf, msg, field->offset, len, field->size_offset,
                         field->alloc_offset, type);

                }else{
                    rMsgArr(rbuf, msg, field->offset, len, field->size_offset,
                            field->alloc_offset, field->schema);
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
                 void *msg, const bor_msg_schema_t *schema)
{
    unsigned char *rbuf;
    rbuf = (unsigned char *)buf;

    borMsgInit(msg, schema);
    decode(&rbuf, msg, schema);
    return 0;
}
