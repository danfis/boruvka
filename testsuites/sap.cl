#define BITS 8u
#define MASK 0xffu
#define NUMS (1 << BITS)
#define NUM_THREADS (1 << BITS)

struct _fer_cd_sap_minmax_t {
    uint geom_ismax;
    float val;
};
typedef struct _fer_cd_sap_minmax_t fer_cd_sap_minmax_t;

union float_uint_t {
    float f;
    uint i;
};

inline uint minmaxPos(__global fer_cd_sap_minmax_t *m,
                      uint shift)
{
    union float_uint_t fi;
    fi.f = m->val;
    fi.i = (fi.i >> shift) & MASK;
    return fi.i;
}

inline void counterZeroize(__global uint *counter, uint offset)
{
    uint i;
    for (i = 0; i < NUMS; i++)
        counter[offset + i] = 0;
}

#if 0
void counterFix(__global uint *counter, uint num_threads)
{
    uint t, i, s, c;

    for (s = 0, c = 0, i = 0; i < NUMS; i++){
        for (t = 0; t < num_threads; t++){
            c += counter[NUMS * t + i];
            counter[NUMS * t + i] = s;
            s = c;
        }
    }
}
#endif

void counterFix1(__global uint *counter, __local uint *local_counter)
{
    int id = get_global_id(0);
    int local_len = get_local_size(0);
    uint from, to, len, i, t, s;

    len = NUMS / local_len;
    from = id * len;
    to   = from + len;
    if (id == local_len - 1)
        to = NUMS;

    for (i = from; i < to; i++){
        s = local_counter[i] = 0;
        for (t = 0; t < local_len; t++){
            local_counter[i] += counter[NUMS * t + i];
            counter[t * NUMS + i] = s;
            s = local_counter[i];
        }
    }
}

void countSort(__global fer_cd_sap_minmax_t *minmax,
               __global uint *counter,
               uint from, uint to, uint offset,
               uint shift)
{
    uint i, pos;

    counterZeroize(counter, offset);

    for (i = from; i < to; i++){
        pos = minmaxPos(&minmax[i], shift);
        ++counter[offset + pos];
    }
}

void countSortFinal(__global fer_cd_sap_minmax_t *minmax,
                    __global uint *counter,
                    __local uint *negative,
                    uint id,
                    uint from, uint to, uint offset,
                    uint shift)
{
    uint i, pos;

    counterZeroize(counter, offset);

    negative[id] = 0;
    for (i = from; i < to; i++){
        pos = minmaxPos(&minmax[i], shift);
        ++counter[offset + pos];

        if (minmax[i].val < 0.f)
            ++negative[id];
    }
}

void sort(__global fer_cd_sap_minmax_t *src,
          __global fer_cd_sap_minmax_t *dst,
          __global uint *counter,
          __local uint *local_counter,
          uint from, uint to, uint offset,
          uint shift)
{
    uint i, pos;

    for (i = from; i < to; i++){
        pos = minmaxPos(&src[i], shift);
        dst[counter[offset + pos] + local_counter[pos]] = src[i];
        ++counter[offset + pos];
    }
}

void sortFinal(__global fer_cd_sap_minmax_t *src,
               __global fer_cd_sap_minmax_t *dst,
               __global uint *counter,
               __local uint *local_counter,
               uint negative, uint minmax_len,
               uint from, uint to, uint offset,
               uint shift)
{
    uint i, val, pos;

    for (i = from; i < to; i++){
        val = minmaxPos(&src[i], shift);

        pos = counter[offset + val] + local_counter[val];
        if (pos >= minmax_len - negative){
            pos = minmax_len - pos - 1;
        }else{
            pos = negative + pos;
        }

        dst[pos] = src[i];
        ++counter[offset + val];
    }
}

__kernel void radixSort(__global fer_cd_sap_minmax_t *_src,
                        __global fer_cd_sap_minmax_t *_dst,
                        __global uint *counter,
                        uint minmax_len)
{
    int id = get_global_id(0);
    int local_id = get_local_id(0);
    int local_len = get_local_size(0);
    __global fer_cd_sap_minmax_t *src, *dst, *mtmp;
    uint from, to, offset, len;
    uint shift, i, j, c, s;
    __local uint local_counter[NUMS];
    __local uint negative[NUM_THREADS];

    len  = minmax_len / local_len;
    from = id * len;
    to   = from + len;
    if (id == local_len - 1)
        to = minmax_len;
    offset = NUMS * id;

    src = _src;
    dst = _dst;

    for (i = 0; i < (32 / BITS) - 1; i++){
        shift = BITS * i;

        countSort(src, counter, from, to, offset, shift);
        barrier(CLK_LOCAL_MEM_FENCE);

        counterFix1(counter, local_counter);
        barrier(CLK_LOCAL_MEM_FENCE);

        if (id == 0){
            c = s = 0;
            for (j = 0; j < NUMS; j++){
                c += local_counter[j];
                local_counter[j] = s;
                s = c;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        sort(src, dst, counter, local_counter, from, to, offset, shift);
        barrier(CLK_LOCAL_MEM_FENCE);

        mtmp = src;
        src = dst;
        dst = mtmp;
    }

    shift = BITS * i;
    countSortFinal(src, counter, negative, id, from, to, offset, shift);
    barrier(CLK_LOCAL_MEM_FENCE);

    counterFix1(counter, local_counter);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (id == 0){
        c = s = 0;
        for (j = 0; j < NUMS; j++){
            c += local_counter[j];
            local_counter[j] = s;
            s = c;
        }

        for (i = 1; i < local_len; i++){
            negative[0] += negative[i];
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    sortFinal(src, dst, counter, local_counter, negative[0], minmax_len, from, to, offset, shift);
}
