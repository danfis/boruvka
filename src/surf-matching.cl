#define SWAP(a, b, tmp) (tmp) = (a); (a) = (b); (b) = (tmp)

static __constant int dim = 64;

float vecDist2(__global float *v, __global float *w)
{
    float dist;
    float f;
    float4 f4;
    __global float4 *v4, *w4;
    int i;

    /* Default version
    dist = 0.f;
    for (i = 0; i < dim; i++){
        f = v[i] - w[i];
        dist += f * f;
    }
    */

    dist = 0.f;
    v4 = (__global float4 *)v;
    w4 = (__global float4 *)w;
    for (i = 0; i < dim / 4; i++){
        f4 = *v4 - *w4;
        dist += dot(f4, f4);
        v4++;
        w4++;
    }

    return dist;
}

__kernel void nearestNeighbor(int len1, __global float *vecs1,
                              int len2, __global float *vecs2,
                              __global float *dist,
                              __global int *ids)
{
    __global float *v1, *v2;
    int id = get_global_id(0);
    int local_id = get_local_id(0);
    int local_len = get_local_size(0);
    int local_end = (get_group_id(0) + 1) * local_len;
    int i;
    float tmp_dist;
    int tmp_ids;
    int window_start, window_end;
    int cycle, row, pos1, pos2;

    if (id < len2){
        v2 = vecs2 + (dim * id);

        // compute distance of first row
        v1 = vecs1;
        ids[id] = id;
        dist[id] = vecDist2(v1, v2);
        barrier(CLK_LOCAL_MEM_FENCE);

        // compute window where this thread will be active
        if (id == local_end - 1){
            window_start = 0;
            window_end   = -1;
        }else{
            window_start = local_len - local_id - 2;
            window_end   = window_start + len1;
        }

        // sort local part of matrix
        row = 0;
        for (cycle = 0; cycle < local_len - 1 + len1; cycle++){
            // compute distance for next row if there is any row left
            if (cycle + 1 < len1){
                v1 = vecs1 + (dim * (cycle + 1));

                ids[len2 * (cycle + 1) + id] = id;
                dist[len2 * (cycle + 1) + id] = vecDist2(v1, v2);
            }

            // if this thread is active sort the 3-element window, where we
            // know that last two elements are sorted from previous thread
            if (cycle >= window_start && cycle < window_end){
                pos1 = len2 * row + id;
                pos2 = pos1 + 1;

                if (dist[pos1] > dist[pos2]){
                    SWAP(dist[pos1], dist[pos2], tmp_dist);
                    SWAP(ids[pos1], ids[pos2], tmp_ids);

                    ++pos1;
                    ++pos2;
                    if (local_id + 2 < local_end && dist[pos1] > dist[pos2]){
                        SWAP(dist[pos1], dist[pos2], tmp_dist);
                        SWAP(ids[pos1], ids[pos2], tmp_ids);
                    }
                }


                row++;
            }

            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }
}
