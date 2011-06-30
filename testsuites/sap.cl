#define BITS 4u
#define MASK 0xfu
#define LEN (1 << BITS)
#define SORT_CTA_SIZE 256
#define WARP_SIZE 32

struct _fer_cd_sap_minmax_t {
    uint geom_ismax;
    float val;
};
typedef struct _fer_cd_sap_minmax_t fer_cd_sap_minmax_t;

union float_uint_t {
    float f;
    uint i;
};

inline uint floatKey(float k)
{
    union float_uint_t fi;
    uint mask;

    fi.f = k;
    mask = -(int)(fi.i >> 31u) | 0x80000000;
    fi.i = fi.i ^ mask;
    return fi.i;
}

inline float floatUnKey(uint k)
{
    union float_uint_t fi;
    uint mask;

    mask = ((k >> 31) - 1) | 0x80000000;
    fi.i = k ^ mask;
    return fi.f;
}


/**
 * @brief Scans one warp quickly, optimized for 32-element warps, using shared memory
 * 
 * Scans each warp in parallel ("warp-scan"), one element per thread.
 * uses 2 numElements of shared memory per thread (64 numElements per warp)
 * 
 * @param[in] val Elements per thread to scan
 * @param[in,out] sData
**/
uint scanwarp(uint val, volatile __local uint* sData, int maxlevel)
{
    // The following is the same as 2 * WARP_SIZE * warpId + threadInWarp = 
    // 64*(threadIdx.x >> 5) + (threadIdx.x & (WARP_SIZE - 1))
    int local_id = get_local_id(0);
    int idx = 2 * local_id - (local_id & (WARP_SIZE - 1));
    sData[idx] = 0;
    idx += WARP_SIZE;
    uint t = sData[idx] = val;

    if (0 <= maxlevel) { sData[idx] = t = t + sData[idx - 1]; }
    if (1 <= maxlevel) { sData[idx] = t = t + sData[idx - 2]; }
    if (2 <= maxlevel) { sData[idx] = t = t + sData[idx - 4]; }
    if (3 <= maxlevel) { sData[idx] = t = t + sData[idx - 8]; }
    if (4 <= maxlevel) { sData[idx] = t = t + sData[idx -16]; }

    return sData[idx] - val;  // convert inclusive -> exclusive
}

/**
 * @brief Scans 4*CTA_SIZE unsigned ints in a block
 *
 * scan4 scans 4*CTA_SIZE numElements in a block (4 per
 * thread), using a warp-scan algorithm
 * 
 * @param[in] idata 4-vector of integers to scan
**/
uint4 scan4(uint4 idata, __local uint* ptr)
{    
    uint idx = get_local_id(0);

    uint4 val4 = idata;
    uint sum[3];
    sum[0] = val4.x;
    sum[1] = val4.y + sum[0];
    sum[2] = val4.z + sum[1];
    
    uint val = val4.w + sum[2];
    
    val = scanwarp(val, ptr, 4);
    barrier(CLK_LOCAL_MEM_FENCE);

    if ((idx & (WARP_SIZE - 1)) == WARP_SIZE - 1){
        ptr[idx >> 5] = val + val4.w + sum[2];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    if (idx < WARP_SIZE){
        ptr[idx] = scanwarp(ptr[idx], ptr, 2);
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    val += ptr[idx >> 5];

    val4.x = val;
    val4.y = val + sum[0];
    val4.z = val + sum[1];
    val4.w = val + sum[2];      
        
    return val4;
}

/**
 * @brief Computes output position for each thread given predicate; trues come first then falses
 * 
 * Rank is the core of the radix sort loop.  Given a predicate, it
 * computes the output position for each thread in an ordering where all
 * True threads come first, followed by all False threads. 
 * This version handles 4 predicates per thread; hence, "rank4".
 *
 * @param[in] preds true/false values for each of the 4 elements in this thread
 *
 * @todo is the description of "preds" correct?
**/
uint4 rank4(uint4 preds, __local uint* shared_mem, __local uint* numtrue)
{
    int local_id = get_local_id(0);
    int cta_size = get_local_size(0);
    uint idx;
    uint4 address, rank;

    address = scan4(preds, shared_mem);
    if (local_id == cta_size - 1){
        numtrue[0] = address.w + preds.w;
    }
	barrier(CLK_LOCAL_MEM_FENCE);

    idx = local_id << 2;
    rank.x = (preds.x) ? address.x : numtrue[0] + idx     - address.x;
    rank.y = (preds.y) ? address.y : numtrue[0] + idx + 1 - address.y;
    rank.z = (preds.z) ? address.z : numtrue[0] + idx + 2 - address.z;
    rank.w = (preds.w) ? address.w : numtrue[0] + idx + 3 - address.w;     
                
    return rank;
}

/**
 * @brief Sorts one block
 *
 * Uses rank to sort one bit at a time: Sorts a block according
 * to bits startbit -> nbits + startbit
 * @param[in,out] key
 * @param[in,out] value
**/
void radixSortBlock(uint4 *key, uint4 *value, uint startbit,
                    __local uint *shared_mem)
{
    uint shift;
    uint4 lsb, r;
    int local_size = get_local_size(0);
    int local_id   = get_local_id(0);
    __local uint numtrue[1];

    for(shift = startbit; shift < (startbit + BITS); ++shift){        
        lsb.x = !(((*key).x >> shift) & 0x1);
        lsb.y = !(((*key).y >> shift) & 0x1);
        lsb.z = !(((*key).z >> shift) & 0x1);
        lsb.w = !(((*key).w >> shift) & 0x1); 

        r = rank4(lsb, shared_mem, numtrue);

        // This arithmetic strides the ranks across 4 local_size regions
        shared_mem[(r.x & 3) * local_size + (r.x >> 2)] = (*key).x;
        shared_mem[(r.y & 3) * local_size + (r.y >> 2)] = (*key).y;
        shared_mem[(r.z & 3) * local_size + (r.z >> 2)] = (*key).z;
        shared_mem[(r.w & 3) * local_size + (r.w >> 2)] = (*key).w; 
        barrier(CLK_LOCAL_MEM_FENCE);

        // The above allows us to read without 4-way bank conflicts:
        (*key).x = shared_mem[local_id];
        (*key).y = shared_mem[local_id +     local_size];
        (*key).z = shared_mem[local_id + 2 * local_size];
        (*key).w = shared_mem[local_id + 3 * local_size];
        barrier(CLK_LOCAL_MEM_FENCE);

        shared_mem[(r.x & 3) * local_size + (r.x >> 2)] = (*value).x;
        shared_mem[(r.y & 3) * local_size + (r.y >> 2)] = (*value).y;
        shared_mem[(r.z & 3) * local_size + (r.z >> 2)] = (*value).z;
        shared_mem[(r.w & 3) * local_size + (r.w >> 2)] = (*value).w;
        barrier(CLK_LOCAL_MEM_FENCE);

        (*value).x = shared_mem[local_id];
        (*value).y = shared_mem[local_id +     local_size];
        (*value).z = shared_mem[local_id + 2 * local_size];
        (*value).w = shared_mem[local_id + 3 * local_size];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

__kernel void radixSortBlocks(__global fer_cd_sap_minmax_t *in,
                              __global fer_cd_sap_minmax_t *out,
                              uint len,
                              uint startbit, uint total_groups,
                              __local uint *shared_mem)
{
    uint group_id = get_group_id(0);
    uint local_size = get_local_size(0);
    uint idx;
    uint4 key, value;

    while (group_id < total_groups){
        // we must compute global id using this because of loop in case
        // there is not enough groups to cover all values
        idx = group_id * local_size + get_local_id(0);

        // each thread process 4 keys/values
        idx = idx << 2;

        // copy keys and values into private memory
        if (idx + 4 <= len){
            key.x = floatKey(in[idx + 0].val);
            key.y = floatKey(in[idx + 1].val);
            key.z = floatKey(in[idx + 2].val);
            key.w = floatKey(in[idx + 3].val);

            value.x = in[idx + 0].geom_ismax;
            value.y = in[idx + 1].geom_ismax;
            value.z = in[idx + 2].geom_ismax;
            value.w = in[idx + 3].geom_ismax;
        }else{
            key.x = (idx     < len ? floatKey(in[idx + 0].val) : UINT_MAX);
            key.y = (idx + 1 < len ? floatKey(in[idx + 1].val) : UINT_MAX);
            key.z = (idx + 2 < len ? floatKey(in[idx + 2].val) : UINT_MAX);
            key.w = UINT_MAX;

            value.x = (idx     < len ? in[idx + 0].geom_ismax : UINT_MAX);
            value.y = (idx + 1 < len ? in[idx + 1].geom_ismax : UINT_MAX);
            value.z = (idx + 2 < len ? in[idx + 2].geom_ismax : UINT_MAX);
            value.w = UINT_MAX;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // sort the block - note that this is already sync'ed
        radixSortBlock(&key, &value, startbit, shared_mem);

        // copy keys and values back to global memory
        if (idx + 4 <= len){
            out[idx + 0].val = floatUnKey(key.x);
            out[idx + 1].val = floatUnKey(key.y);
            out[idx + 2].val = floatUnKey(key.z);
            out[idx + 3].val = floatUnKey(key.w);

            out[idx + 0].geom_ismax = value.x;
            out[idx + 1].geom_ismax = value.y;
            out[idx + 2].geom_ismax = value.z;
            out[idx + 3].geom_ismax = value.w;
        }else{
            if (idx < len){
                out[idx + 0].val = floatUnKey(key.x);
                out[idx + 0].geom_ismax = value.x;

                if (idx + 1 < len){
                    out[idx + 1].val = floatUnKey(key.y);
                    out[idx + 1].geom_ismax = value.y;

                    if (idx + 2 < len){
                        out[idx + 2].val = floatUnKey(key.z);
                        out[idx + 2].geom_ismax = value.z;
                    }
                }
            }
        }

        if (get_num_groups(0) < total_groups){
            group_id += get_num_groups(0);
        }else{
            break;
        }
    }
}


/** @brief Computes the number of keys of each radix in each block stores offset.
*
* Given an array with blocks sorted according to a 4-bit radix group, each 
* block counts the number of keys that fall into each radix in the group, and 
* finds the starting offset of each radix in the block.  It then writes the radix 
* counts to the counters array, and the starting offsets to the blockOffsets array.
*
* Template parameters are used to generate efficient code for various special cases
* For example, we have to handle arrays that are a multiple of the block size 
* (fullBlocks) differently than arrays that are not. "loop" is used when persistent 
* CTAs are used. 
*
* By persistent CTAs we mean that we launch only as many thread blocks as can 
* be resident in the GPU and no more, rather than launching as many threads as
* we have elements. Persistent CTAs loop over blocks of elements until all work
* is complete.  This can be faster in some cases.  In our tests it is faster
* for large sorts (and the threshold is higher on compute version 1.1 and earlier
* GPUs than it is on compute version 1.2 GPUs.
* 
* @param[in] keys Input keys
* @param[out] counters Radix count for each block
* @param[out] blockOffsets The offset address for each block
* @param[in] numElements Total number of elements
* @param[in] totalBlocks Total number of blocks
**/
__kernel void radixSortOffsets(__global fer_cd_sap_minmax_t *keys, 
                               __global uint *counters, 
                               __global uint *block_offsets, 
                               uint len,
                               uint startbit,
                               uint total_groups,
                               __local uint *shared_mem)
{
    uint group_id   = get_group_id(0);
    uint local_size = get_local_size(0);
    uint local_id   = get_local_id(0);
    uint idx;
    uint2 radix2;
    __local uint start_pointers[16];

    while (group_id < total_groups){
        // compute start idx from group_id which can change in next
        // iteration
        idx = group_id * local_size + local_id;

        // each thread process two keys
        idx = idx << 1;

        // store keys in private memory
        if (idx + 1 < len){
            radix2.x = floatKey(keys[idx    ].val);
            radix2.y = floatKey(keys[idx + 1].val);
        }else{
            radix2.x = (idx     < len ? floatKey(keys[idx    ].val) : UINT_MAX);
            radix2.y = (idx + 1 < len ? floatKey(keys[idx + 1].val) : UINT_MAX);
        }

        shared_mem[local_id << 1      ] = (radix2.x >> startbit) & MASK;
        shared_mem[(local_id << 1) + 1] = (radix2.y >> startbit) & MASK;

        // Finds the position where the shared_mem entries differ and stores start 
        // index for each radix.
        if(local_id < 16){ 
            start_pointers[local_id] = 0; 
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // mark start pointers
        if(local_id > 0 && (shared_mem[local_id] != shared_mem[local_id - 1])){
            start_pointers[shared_mem[local_id]] = local_id;
        }
        if(shared_mem[local_id + local_size] != shared_mem[local_id + local_size - 1]){
            start_pointers[shared_mem[local_id + local_size]] = local_id + local_size;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // write start pointers as block offsets
        if(local_id < 16){
            block_offsets[group_id * 16 + local_id] = start_pointers[local_id];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // Compute the sizes of each block.
        if(local_id > 0 && (shared_mem[local_id] != shared_mem[local_id - 1])){
            start_pointers[shared_mem[local_id - 1]] =
                local_id - start_pointers[shared_mem[local_id - 1]];
        }
        if(shared_mem[local_id + local_size] != shared_mem[local_id + local_size - 1]){
            start_pointers[shared_mem[local_id + local_size - 1]] = 
                local_id + local_size - start_pointers[shared_mem[local_id + local_size - 1]];
        }

        if(local_id == local_size - 1){
            start_pointers[shared_mem[2 * local_size - 1]] = 
                2 * local_size - start_pointers[shared_mem[2 * local_size - 1]];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // sort counters to have first all zeroes, then all ones etc...
        if(local_id < 16){
            counters[local_id * total_groups + group_id] = start_pointers[local_id];
        }

        /*
        if(local_id < 16){ 
            counters[group_id * 16 + local_id] = start_pointers[local_id];
        }
        */
        barrier(CLK_LOCAL_MEM_FENCE);

        if (get_num_groups(0) < total_groups){
            group_id += get_num_groups(0);
        }else{
            break;
        }
    }
}


/**@brief Reorders data in the global array.
*
* reorderData shuffles data in the array globally after the radix
* offsets have been found. On compute version 1.1 and earlier GPUs, this code depends 
* on SORT_CTA_SIZE being 16 * number of radices (i.e. 16 * 2^nbits).
* 
* On compute version 1.1 GPUs ("manualCoalesce=true") this function ensures
* that all writes are coalesced using extra work in the kernel.  On later
* GPUs coalescing rules have been relaxed, so this extra overhead hurts 
* performance.  On these GPUs we set manualCoalesce=false and directly store
* the results.
*
* Template parameters are used to generate efficient code for various special cases
* For example, we have to handle arrays that are a multiple of the block size 
* (fullBlocks) differently than arrays that are not.  "loop" is used when persistent 
* CTAs are used. 
*
* By persistent CTAs we mean that we launch only as many thread blocks as can 
* be resident in the GPU and no more, rather than launching as many threads as
* we have elements. Persistent CTAs loop over blocks of elements until all work
* is complete.  This can be faster in some cases.  In our tests it is faster
* for large sorts (and the threshold is higher on compute version 1.1 and earlier
* GPUs than it is on compute version 1.2 GPUs.
*
* @param[out] outKeys Output of sorted keys 
* @param[out] outValues Output of associated values 
* @param[in] keys Input of unsorted keys in GPU 
* @param[in] values Input of associated input values 
* @param[in] blockOffsets The offset address for each block
* @param[in] offsets Address of each radix within each block
* @param[in] sizes Number of elements in a block
* @param[in] numElements Total number of elements
* @param[in] totalBlocks Total number of data blocks to process
*
* @todo Args that are const below should be prototyped as const
**/
__kernel void radixSortReorder(__global fer_cd_sap_minmax_t *in,
                               __global fer_cd_sap_minmax_t *out,
                               __global uint *g_counters,
                               __global uint *g_offsets,
                               uint len,
                               uint startbit,
                               uint total_groups)
{
    __local uint counters[16];
    __local uint offsets[16];
    fer_cd_sap_minmax_t keyval;
    uint key, pos;
    uint group_id   = get_group_id(0);
    uint local_size = get_local_size(0);
    uint local_id   = get_local_id(0);
    uint idx, lidx;


    while (group_id < total_groups){
        // copy counters and offsets to local memory
        if (local_id < 16){
            counters[local_id] = g_counters[local_id * total_groups + group_id];
        }else if (local_id >= 16 && local_id < 32){
            offsets[local_id - 16] = g_offsets[group_id * 16 + (local_id - 16)];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        idx = group_id * local_size + local_id;
        idx = idx << 1;
        lidx = local_id << 1;

        if (idx < len){
            keyval = in[idx];
            key = floatKey(keyval.val);
            key = (key >> startbit) & MASK;
            pos = counters[key] + (lidx - offsets[key]);

            out[pos] = keyval;
            //out[idx].val = offsets[key];
            //out[idx].geom_ismax = key;
        }

        ++idx;
        ++lidx;
        if (idx < len){
            keyval = in[idx];
            key = floatKey(keyval.val);
            key = (key >> startbit) & MASK;
            pos = counters[key] + (lidx - offsets[key]);

            out[pos] = keyval;
            //out[idx].val = offsets[key];
            //out[idx].geom_ismax = key;
        }

        if (get_num_groups(0) < total_groups){
            group_id += get_num_groups(0);
            barrier(CLK_LOCAL_MEM_FENCE);
        }else{
            break;
        }
    }
}

__kernel void radixSortPrescan(__global uint *in,
                               __global uint *out,
                               __global uint *sums,
                               uint len, uint total_groups,
                               __local uint *temp)
{
    uint group_id = get_group_id(0);
    uint local_size = get_local_size(0);
    uint idx;
    uint4 key;

    while (group_id < total_groups){
        // we must compute global id using this because of loop in case
        // there is not enough groups to cover all values
        idx = group_id * local_size + get_local_id(0);

        // each thread process 4 keys/values
        idx = idx << 2;

        // copy keys and values into private memory
        if (idx + 4 <= len){
            key = ((__global uint4 *)in)[idx >> 2];
        }else{
            key.x = (idx     < len ? in[idx + 0] : 0);
            key.y = (idx + 1 < len ? in[idx + 1] : 0);
            key.z = (idx + 2 < len ? in[idx + 2] : 0);
            key.w = 0;
        }

        if (sums && get_local_id(0) == local_size - 1){
            sums[group_id] = key.w;
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        // sort the block - note that this is already sync'ed
        key = scan4(key, temp);

        if (sums && get_local_id(0) == local_size - 1){
            sums[group_id] += key.w;
        }

        // copy keys and values back to global memory
        if (idx + 4 <= len){
            ((__global uint4 *)out)[idx >> 2] = key;
        }else{
            if (idx < len){
                out[idx + 0] = key.x;

                if (idx + 1 < len){
                    out[idx + 1] = key.y;

                    if (idx + 2 < len){
                        out[idx + 2] = key.z;
                    }
                }
            }
        }


        if (get_num_groups(0) < total_groups){
            group_id += get_num_groups(0);
            barrier(CLK_LOCAL_MEM_FENCE);
        }else{
            break;
        }
    }
}

__kernel void radixSortPrescanFinish(__global uint *in,
                                     __global uint *out,
                                     __global uint *sums,
                                     uint len, uint total_groups)
{
    uint group_id = get_group_id(0);
    uint local_size = get_local_size(0);
    uint idx;
    __local sadd[1];
    uint4 key, add;

    group_id += 1;
    while (group_id < total_groups){
        if (get_local_id(0) == 0){
            sadd[0] = sums[group_id];
        }
        barrier(CLK_LOCAL_MEM_FENCE);

        add.x = sadd[0];
        add.w = add.z = add.y = add.x;

        // we must compute global id using this because of loop in case
        // there is not enough groups to cover all values
        idx = group_id * local_size + get_local_id(0);

        // each thread process 4 keys/values
        idx = idx << 2;

        // copy keys and values into private memory
        if (idx + 4 <= len){
            key = ((__global uint4 *)in)[idx >> 2];
        }else{
            key.x = (idx     < len ? in[idx + 0] : 0);
            key.y = (idx + 1 < len ? in[idx + 1] : 0);
            key.z = (idx + 2 < len ? in[idx + 2] : 0);
            key.w = 0;
        }
        key += add;


        // copy keys and values back to global memory
        if (idx + 4 <= len){
            ((__global uint4 *)out)[idx >> 2] = key;
        }else{
            if (idx < len){
                out[idx + 0] = key.x;

                if (idx + 1 < len){
                    out[idx + 1] = key.y;

                    if (idx + 2 < len){
                        out[idx + 2] = key.z;
                    }
                }
            }
        }


        if (get_num_groups(0) < total_groups){
            group_id += get_num_groups(0);
            barrier(CLK_LOCAL_MEM_FENCE);
        }else{
            break;
        }
    }
}
