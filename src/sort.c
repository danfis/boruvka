/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <fermat/sort.h>
#include <fermat/dbg.h>


/**** RADIX SORT ****/
#define RADIX_SORT_MASK 0xffu

_fer_inline uint32_t radixSortVal(const fer_radix_sort_t *m,
                                  uint32_t shift)
{
    fer_uint_t val;

    val = ferRealAsUInt(m->key);
    val = (val >> shift) & RADIX_SORT_MASK;

    return val;
}

_fer_inline void radixSortZeroizeCounter(uint32_t *counter)
{
    int i;
    for (i = 0; i < 256; i++)
        counter[i] = 0;
}

static void radixSortCount(const fer_radix_sort_t *m,
                           uint32_t len,
                           uint32_t *counter, uint32_t shift)
{
    uint32_t i;
    fer_uint_t val;

    radixSortZeroizeCounter(counter);
    for (i = 0; i < len; i++){
        val = radixSortVal(&m[i], shift);
        ++counter[val];
    }
}

static void radixSortCountFinal(const fer_radix_sort_t *m,
                                uint32_t len,
                                uint32_t *counter,
                                uint32_t *negative,
                                uint32_t shift)
{
    uint32_t i;
    fer_uint_t val;

    radixSortZeroizeCounter(counter);
    *negative = 0;
    for (i = 0; i < len; i++){
        val = radixSortVal(&m[i], shift);
        ++counter[val];

        if (m[i].key < FER_ZERO)
            ++(*negative);
    }
}

static void radixSortFixCounter(uint32_t *counter)
{
    uint32_t i, val, t;

    val = 0;
    for (i = 0; i < 256; i++){
        t = counter[i];
        counter[i] = val;
        val += t;
    }
}

static void radixSortSort(fer_radix_sort_t *src,
                          fer_radix_sort_t *dst,
                          uint32_t len,
                          uint32_t *counter,
                          uint32_t shift)
{
    uint32_t i;
    fer_uint_t val;

    for (i = 0; i < len; i++){
        val = radixSortVal(&src[i], shift);
        dst[counter[val]] = src[i];
        ++counter[val];
    }
}

static void radixSortSortFinal(fer_radix_sort_t *src,
                               uint32_t srclen,
                               fer_radix_sort_t *dst,
                               uint32_t dstlen,
                               uint32_t *counter,
                               uint32_t negative,
                               uint32_t shift)
{
    uint32_t i;
    fer_uint_t val, pos;

    for (i = 0; i < srclen; i++){
        val = radixSortVal(&src[i], shift);
        pos = counter[val];
        if (pos >= dstlen - negative){
            pos = dstlen - pos - 1;
        }else{
            pos = negative + pos;
        }

        dst[pos] = src[i];

        ++counter[val];
    }
}

void ferRadixSort(fer_radix_sort_t *rs, fer_radix_sort_t *rs_tmp, size_t rslen)
{
    fer_radix_sort_t *src, *dst, *tmp;
    uint32_t shift, i, len;
    uint32_t counter[RADIX_SORT_MASK], negative;


    len = (fer_uint_t)sizeof(fer_real_t) - 1;
    shift = 0;
    src = rs;
    dst = rs_tmp;
    for (i = 0; i < len; i++){
        radixSortCount(src, rslen, counter, shift);
        radixSortFixCounter(counter);
        radixSortSort(src, dst, rslen, counter, shift);

        shift += 8;
        FER_SWAP(src, dst, tmp);
    }

    radixSortCountFinal(src, rslen, counter, &negative, shift);
    radixSortFixCounter(counter);
    radixSortSortFinal(src, rslen, dst, rslen, counter, negative, shift);
}

/**** RADIX SORT END ****/
