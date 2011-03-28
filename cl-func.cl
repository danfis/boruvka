int nearestInRadiusCellID(int global_id, int global_size,
                          int d,
                          int radius,
                          __global size_t *center,
                          __global size_t *dim)
{
    int threads_per_side = global_size / (2 * d);
    int id = global_id % threads_per_side;
    int fix, neg_radius;
    int i, coord, outid, mul;

    fix = global_id / (threads_per_side * 2);
    neg_radius = (global_id / threads_per_side) % 2;

    outid = 0;
    mul = 1;

    // iterate over coordinates
    for (i = 0; i < d; i++){
        if (i == fix){
            // this the fixed coordinate
            if (neg_radius){
                coord = -radius;
            }else{
                coord = radius;
            }
        }else{
            // compute i'th coordinate from id and alter id
            coord = id % (2 * radius + 1);
            coord = coord - radius;
            id = id / (2 * radius + 1);
        }

        // check if are not outside a bounds
        if (i < fix && (coord == radius || coord == -radius)){
            outid = -2;
            break;
        }

        // translate coordinate to global coordinates
        coord += center[i];

        // check if we are outside cells
        if (coord >= (int)dim[i] || coord < 0){
            outid = -1;
            break;
        }

        outid += coord * mul;
        mul *= dim[i];
    }

    return outid;
}

float dist2(int d, __global float *p, __global float *o)
{
    float dist, v;
    int i;

    dist = 0.f;

    for (i = 0; i < d; i++){
        v     = p[i] - o[i];
        dist += v * v;
    }

    return dist;
}

void insertDist(__global float *_dist, __global int *_ids,
                int dist_pos, int num_neighbors,
                float val, int id)
{
    __global float *dist, *dist_stop;
    __global int *ids;

    dist_stop = _dist + dist_pos;
    dist      = dist_stop + num_neighbors;
    ids       = _ids + dist_pos + num_neighbors;
    if (*dist > val){
        *dist = val;
        *ids  = id;
        dist--;
        ids--;
        while (dist >= dist_stop && *dist > val){
            *(dist + 1) = *dist;
            *dist = val;
            *(ids + 1) = *ids;
            *ids  = id;
            dist--;
            ids--;
        }
    }
}

void bubbleUp(__global float *from, __global int *ids, __global float *stop)
{
    float val;
    int id;

    while (from > stop && *from < *(from - 1)){
        val = *from;
        *from = *(from - 1);
        *(from - 1) = val;

        id = *ids;
        *ids = *(ids - 1);
        *(ids - 1) = id;

        from--;
        ids--;
    }
}

void merge(__global float *_dist, __global int *_ids, int dist_pos, int length, int distlen,
        __global float *debug, int id)
{
    __global float *left_stop, *right, *right_stop;
    __global int *ids;

    if (distlen - dist_pos > length){
        //debug[id] = (distlen - dist_pos) * 1000 + length;

        left_stop  = _dist + dist_pos;
        right      = left_stop + length;
        right_stop = right + length;
        ids        = _ids + dist_pos + length;
        if (distlen - dist_pos - length < length){
            right_stop = right + (distlen - dist_pos - length);
        }


        //debug[id] = *right;

        while (*right < *(right - 1) && right <= right_stop){
            bubbleUp(right, ids, left_stop);
            right++;
            ids++;
        }
    }
}

__kernel void nearestInRadiusCells(int d,
                                   int radius,
                                   int max_els_per_cell,
                                   int num_neighbors,
                                   __global size_t *center,
                                   __global size_t *dim,
                                   __global float *p,
                                   __global float *coords,
                                   __global int *els,
                                   __global float *dist,
                                   __global int *ids,
                                   __global float *debug)
{
    int ids_pos = get_global_id(0);
    int dist_pos = get_global_id(0) * num_neighbors;
    int dist_len, active_threads, rank_len, overall_len;
    int i, cell_id, els_i, els_max, coords_i;
    float vdist;

    // reset dist array
    for (i = 0; i < num_neighbors; i++){
        dist[dist_pos + i] = FLT_MAX;
        ids[dist_pos + i]  = -1;
    }
    debug[ids_pos] = 99999;
    //ids[ids_pos] = ids_pos;
    //ids[ids_pos] = nearestInRadiusCellID(get_global_id(0), get_global_size(0), d, radius, center, dim);

    // compute id of cell that will be searched
    cell_id = nearestInRadiusCellID(get_global_id(0), get_global_size(0), d, radius, center, dim);
    if (cell_id >= 0){
        // start and end of array of elements that belongs to the cell
        els_i   = cell_id * max_els_per_cell;
        els_max = els_i + max_els_per_cell;

        // compute distances from point p and store it in dist array
        while (els[els_i] >= 0 && els_i < els_max){
            coords_i = els[els_i] * d;
            vdist    = dist2(d, p, coords + coords_i);
            insertDist(dist, ids, dist_pos, num_neighbors, vdist, els[els_i]);

            debug[ids_pos] = els[els_i] * 1000 + cell_id + 1000000;
            debug[ids_pos] = 1;
            els_i++;
        }
    }

    active_threads = 1;
    barrier(CLK_LOCAL_MEM_FENCE);

    overall_len = num_neighbors * get_global_size(0);
    do {
        active_threads *= 2;
        rank_len = num_neighbors * (active_threads / 2);

        if (get_global_id(0) % active_threads == 0){
            merge(dist, ids, dist_pos, rank_len, overall_len, debug,
                    get_global_id(0));

            //debug[get_global_id(0)] = 100 * active_threads;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    //} while(0);
    } while (2 * rank_len < overall_len);
}
