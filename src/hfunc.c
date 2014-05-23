/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/hfunc.h>



#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}


#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

uint32_t borHashJenkins(const uint32_t *k, size_t length, uint32_t initval)
{
    uint32_t a,b,c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32_t)length)<<2) + initval;

    /*------------------------------------------------- handle most of the key */
    while (length > 3)
    {
        a += k[0];
        b += k[1];
        c += k[2];
        mix(a,b,c);
        length -= 3;
        k += 3;
    }

    /*------------------------------------------- handle the last 3 uint32_t's */
    switch(length)                     /* all the case statements fall through */
    { 
        case 3 : c+=k[2];
        case 2 : b+=k[1];
        case 1 : a+=k[0];
                 final(a,b,c);
        case 0:     /* case 0: nothing left to add */
                 break;
    }
    /*------------------------------------------------------ report the result */
    return c;
}


uint32_t borHashDJB2(const char *str)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *str++) != 0x0){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

uint32_t borHashSDBM(const char *str)
{
    uint32_t hash = 0;
    int c;

    while ((c = *str++) != 0){
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}


#define FNV1A_32_INIT ((uint32_t)0x811c9dc5ULL)
#define FNV1A_64_INIT ((uint64_t)0xcbf29ce484222325ULL)

uint32_t borFnv1a_32(const void *_buf, size_t size)
{
    unsigned char *buf = (unsigned char *)_buf;
    unsigned char *end = buf + size;
    uint32_t val = FNV1A_32_INIT;

    while (buf < end){
        val ^= (uint32_t)*buf++;
        val += (val<<1) + (val<<4) + (val<<7) + (val<<8) + (val<<24);
    }

    return val;
}


uint64_t borFnv1a_64(const void *_buf, size_t size)
{
    unsigned char *buf = (unsigned char *)_buf;
    unsigned char *end = buf + size;
    uint64_t val = FNV1A_64_INIT;

    while (buf < end){
        val ^= (uint64_t)*buf++;
        val += (val << 1) + (val << 4) + (val << 5) +
		       (val << 7) + (val << 8) + (val << 40);
    }

    return val;
}


#define MURMUR3_SEED 3145739

_bor_inline uint32_t rotl32(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

_bor_inline uint32_t fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

uint32_t borMurmur3_32(const void *_buf, size_t size)
{
    const unsigned char *buf = (const unsigned char *)_buf;
    int num_blocks = size / 4;
    const uint32_t *blocks = (const uint32_t *)(buf + num_blocks*4);
    const unsigned char* tail = (const unsigned char *)(buf + num_blocks*4);
    uint32_t h1 = MURMUR3_SEED;
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    int i;

    for (i = -num_blocks; i; ++i){
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = rotl32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1,13); 
        h1 = h1*5+0xe6546b64;
    }


    uint32_t k1 = 0;

    switch (size & 3){
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
    }

    h1 ^= size;
    h1 = fmix32(h1);

    return h1;
}
