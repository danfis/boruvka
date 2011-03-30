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
                              __global float *gdist,
                              __global int *gids)
{
    __local float dist[3][256];
    __local int ids[3][256];
    __global float *v1, *v2;
    int id = get_global_id(0);
    int local_id = get_local_id(0);
    int local_len = get_local_size(0);
    int delay, cycle, cycle_end, row, pos;
    float tmp_dist;
    int tmp_ids;


    delay = local_len - local_id - 1;
    cycle_end = local_len - 1 + len1;
    row = 0;
    for (cycle = 0; cycle < cycle_end; cycle++){
        if (cycle >= delay && cycle < delay + len1){
            // write number on position
            v2 = vecs2 + (dim * id);
            v1 = vecs1 + (dim * row);

            dist[row % 3][local_id] = vecDist2(v1, v2);
            ids[row % 3][local_id]  = id;


            // sort 2 or 3 element window
            if (local_id < local_len - 1){
                if (dist[row % 3][local_id] > dist[row % 3][local_id + 1]){
                    SWAP(dist[row % 3][local_id], dist[row % 3][local_id + 1], tmp_dist);
                    SWAP(ids[row % 3][local_id], ids[row % 3][local_id + 1], tmp_ids);

                    if (local_id < local_len - 2
                            && dist[row % 3][local_id + 1] > dist[row % 3][local_id + 2]){
                        SWAP(dist[row % 3][local_id + 1], dist[row % 3][local_id + 2], tmp_dist);
                        SWAP(ids[row % 3][local_id + 1], ids[row % 3][local_id + 2], tmp_ids);
                    }
                }
            }

            // we have sorted this row - write it to global array
            if (local_id == 0){
                pos = row * get_num_groups(0) * 2 + get_group_id(0) * 2;
                gdist[pos]     = dist[row % 3][0];
                gdist[pos + 1] = dist[row % 3][1];
                gids[pos]      = ids[row % 3][0];
                gids[pos + 1]  = ids[row % 3][1];
            }

            row += 1;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }
}
