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
    uint mask;
    fi.f = m->val;

    mask = -(int)(fi.i >> 31u) | 0x80000000;
    fi.i = fi.i ^ mask;
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
