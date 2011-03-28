
float vecDist2(int dim, __global float *v, __global float *w)
{
    float f, dist;
    int i;

    f    = v[0] - w[0];
    dist = f * f;
    for (i = 1; i < dim; i++){
        f     = v[i] - w[i];
        dist += f * f;
    }

    return dist;
}

/** Distance of float64 */
float vecDist2_64(__global float *v, __global float *w)
{
    float dist;
    float4 f;
    int i;

    dist = 0;
    for (i = 0; i < 16; i++){
        f = *((float4 *)(v + (i * 4))) - *((float4 *)(w + (i * 4)));
        dist += dot(f, f);
        //f += dot(*((float4 *)(v + (i * 4))), *((float4 *)(w + (i * 4))));
    }

    return dist;
}

void mergeBubbleLeft(__global float *right, __global float *left,
                     __global int *ids_right)
{
    float tmp;
    int ids_tmp;

    tmp = *right;
    ids_tmp = *ids_right;
    while (right > left){
        *right = *(right - 1);
        right--;

        *ids_right = *(ids_right - 1);
        ids_right--;
    }
    *right = tmp;
    *ids_right = ids_tmp;
    
}

void merge(int start, int range, int len, int k,
           __global float *dists, __global int *ids)
{
    __global float *left, *right, *right_end;
    __global int *ids_right;
    int count;
    float tmp;
    int ids_tmp;

    if (start + range < len){
        if (range == 1){
            if (start + 1 < len && dists[start] > dists[start + 1]){
                tmp = dists[start];
                dists[start] = dists[start + 1];
                dists[start + 1] = tmp;

                ids_tmp = ids[start];
                ids[start] = ids[start + 1];
                ids[start + 1] = ids_tmp;
            }
        }else{
            left      = dists + start;
            right     = left + range;
            right_end = right + range;
            if (right_end > dists + len){
                right_end = dists + len;
            }
            ids_right = ids + start + range;

            count = 0;
            while (count < k && left < right && right < right_end){
                if (*left <= *right){
                    left++;
                }else{
                    mergeBubbleLeft(right, left, ids_right);
                    left++;
                    right++;
                    ids_right++;
                }

                count++;
            }
        }
    }
}

/** Merge in case we want two nearest neighbors */
void merge_2(int start, int range, int len,
             __global float *dists, __global int *ids)
{
    __global float *left, *right, *right_end;
    __global int *ids_left, *ids_right;
    int i;
    float tmp;
    int ids_tmp;

    if (start + range < len){
        if (range == 1){
            if (start + 1 < len && dists[start] > dists[start + 1]){
                tmp = dists[start];
                dists[start] = dists[start + 1];
                dists[start + 1] = tmp;

                ids_tmp = ids[start];
                ids[start] = ids[start + 1];
                ids[start + 1] = ids_tmp;
            }
        }else{
            left      = dists + start;
            right     = left + range;
            ids_left  = ids + start;
            ids_right = ids_left + range;

            for (i = 0; i < 2 && start + range + i < len; i++, right++, ids_right++){
                if (*right < *left){
                    *(left + 1) = *left;
                    *left = *right;

                    *(ids_left + 1) = *ids_left;
                    *ids_left = *ids_right;
                }else if (*right < *(left + 1)){
                    *(left + 1) = *right;
                    *(ids_left + 1) = *ids_right;
                }
            }
        }
    }
}

__kernel void nearestNeighbor(int dim,
                              int len,
                              int k,
                              __global float *vecs,
                              __global float *p,
                              __global float *dists,
                              __global int *ids)
{
    int glob_id = get_global_id(0);
    int sort_range;

    if (glob_id < len){
        // compute distance between vectors
        dists[glob_id] = vecDist2(dim, vecs + (glob_id * dim), p);
        ids[glob_id]   = glob_id;
        barrier(CLK_LOCAL_MEM_FENCE);

        // sort
        sort_range = 1;

        do {
            if (glob_id % (2 * sort_range) == 0){
                merge(glob_id, sort_range, len, k, dists, ids);
            }

            sort_range *= 2;
            barrier(CLK_LOCAL_MEM_FENCE);
        } while (sort_range < len);
    }
}


/**
 * Two nearest neighbors on 64 long vectors.
 */
__kernel void nearestNeighbor_2_64(int len,
                                   __global float *vecs,
                                   __global float *p,
                                   __global float *dists,
                                   __global int *ids)
{
    int glob_id = get_global_id(0);
    int sort_range;

    if (glob_id < len){
        // compute distance between vectors
        dists[glob_id] = vecDist2_64(vecs + (glob_id * 64), p);
        ids[glob_id]   = glob_id;
        barrier(CLK_LOCAL_MEM_FENCE);

        // sort
        sort_range = 1;

        do {
            if (glob_id % (2 * sort_range) == 0){
                merge_2(glob_id, sort_range, len, dists, ids);
            }

            sort_range *= 2;
            barrier(CLK_LOCAL_MEM_FENCE);
        } while (sort_range < len);
    }
}
