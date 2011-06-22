#define BITS 4u
#define MASK 0xfu
#define LEN (1 << BITS)

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

__kernel void radixSortReduce(__global fer_cd_sap_minmax_t *_src,
                              uint minmax_len,
                              __global uint *__global_counter,
                              __global uint *global_counter_sum,
                              uint shift)
{
    uint id        = get_global_id(0);
    uint local_id  = get_local_id(0);
    uint i, from, len, pos, j;
    __global fer_cd_sap_minmax_t *src;
    __global uint *global_counter;
    __local uint local_counter[LEN * LEN];
    __local uint *counter = local_counter + (local_id * LEN);

    // compute length of section managed by this thread and strating
    // position ({from})
    len  = minmax_len / get_global_size(0);
    from = id * len;
    if (id == get_global_size(0) - 1)
        len = minmax_len - from;

    // zeroize counter
    for (i = 0; i < LEN; i++)
        counter[i] = 0;

    // run count sort on local counter
    src = _src + from;
    for (i = 0; i < len; i++){
        pos = minmaxPos(src + i, shift);
        ++counter[pos];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // reduce colums of counter and copy that into global memory
    global_counter = __global_counter + (LEN * LEN * get_group_id(0));

    global_counter_sum[id] = 0;
    for (i = local_id, j = 0; i < LEN * LEN; i += LEN, j++){
        global_counter[i] = local_counter[i];
        global_counter_sum[id] += local_counter[i];
    }
}



__kernel void radixSortFixCounter1(__global uint *global_counter_sum,
                                   uint num_groups)
{
    uint id        = get_global_id(0);
    uint local_id  = get_local_id(0);
    uint add, i, len, tmp;

    if (get_group_id(0) == 0){
        // sum values in columns
        len = LEN * num_groups;
        for (i = local_id + LEN; i < len; i += LEN){
            global_counter_sum[i] += global_counter_sum[i - LEN];
        }
        barrier(CLK_GLOBAL_MEM_FENCE);

        // sum values in last section
        if (id == 0){
            add = global_counter_sum[(LEN * (num_groups - 1))];
            len = LEN * num_groups;
            for (i = LEN * (num_groups - 1) + 1; i < len; ++i){
                tmp = global_counter_sum[i];
                global_counter_sum[i] += add;
                add += tmp;
            }
        }
        barrier(CLK_GLOBAL_MEM_FENCE);

        // fix sum values in columns
        if (local_id != 0){
            add = global_counter_sum[(LEN * (num_groups - 1)) + local_id- 1];
            len = LEN * (num_groups - 1);
            for (i = local_id; i < len; i += LEN){
                global_counter_sum[i] += add;
            }
        }
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
}

__kernel void radixSortFixCounter2(__global uint *__global_counter,
                                   __global uint *global_counter_sum)
{
    uint id        = get_global_id(0);
    uint local_id  = get_local_id(0);
    __global uint *global_counter;
    uint add, i, len, tmp;

    global_counter = __global_counter + (LEN * LEN * get_group_id(0));

    add = 0;
    if (get_group_id(0) > 0){
        add = global_counter_sum[id - LEN];
    }else if (local_id != 0){
        add = global_counter_sum[(LEN * (get_num_groups(0) - 1)) + local_id - 1];
    }

    len = LEN * LEN + local_id;
    for (i = local_id; i < len; i += LEN){
        tmp = global_counter[i];
        global_counter[i] = add;
        add += tmp;
    }
}

__kernel void radixSortCopy(__global fer_cd_sap_minmax_t *_src,
                            __global fer_cd_sap_minmax_t *dst,
                            uint minmax_len,
                            __global uint *__global_counter,
                            uint shift)
{
    uint id        = get_global_id(0);
    uint local_id  = get_local_id(0);
    uint from, len, i, val, pos;
    __global fer_cd_sap_minmax_t *src;
    __global uint *global_counter;
    uint counter[LEN];

    len  = minmax_len / get_global_size(0);
    from = id * len;
    if (id == get_global_size(0) - 1)
        len = minmax_len - from;

    // copy counterto private memory
    global_counter  = __global_counter + LEN * id;
    for (i = 0; i < LEN; i++)
        counter[i] = global_counter[i];

    // place minmax values in its place
    src = _src + from;
    for (i = 0; i < len; i++){
        val = minmaxPos(src + i, shift);
        pos = counter[val];

        dst[pos] = src[i];

        ++counter[val];
    }
}

#if 0
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
                        __global uint *global_counter,
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
    uint counter[NUMS];

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

        countSort(src, global_counter, from, to, offset, shift);
        /*
        countSort1(src, counter, from, to, shift);
        for (j = 0; j < NUMS; j++){
            global_counter[offset + j] = counter[j];
        }
        */
        barrier(CLK_GLOBAL_MEM_FENCE);

        counterFix1(global_counter, local_counter);
        barrier(CLK_GLOBAL_MEM_FENCE);

        if (id == 0){
            c = s = 0;
            for (j = 0; j < NUMS; j++){
                c += local_counter[j];
                local_counter[j] = s;
                s = c;
            }
        }
        barrier(CLK_GLOBAL_MEM_FENCE);

        sort(src, dst, global_counter, local_counter, from, to, offset, shift);
        barrier(CLK_GLOBAL_MEM_FENCE);

        mtmp = src;
        src = dst;
        dst = mtmp;
    }

    shift = BITS * i;
    countSortFinal(src, global_counter, negative, id, from, to, offset, shift);
    barrier(CLK_GLOBAL_MEM_FENCE);

    counterFix1(global_counter, local_counter);
    barrier(CLK_GLOBAL_MEM_FENCE);

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
    barrier(CLK_GLOBAL_MEM_FENCE);

    sortFinal(src, dst, global_counter, local_counter, negative[0], minmax_len, from, to, offset, shift);
}
#endif
