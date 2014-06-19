#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include "boruvka/alloc.h"
#include "boruvka/bucketheap.h"

/** Maximal key the bucket-heap will accept.
 *  This value serve as a protection from excessively high memory
 *  allocations which can come when a bucket-heap is used unwisely. */
#define BOR_BUCKETHEAP_EXCESSIVELY_HIGH_KEY (1024u * 1024u * 1024u)

static size_t pagesize(void)
{
    return sysconf(_SC_PAGESIZE);
}

bor_bucketheap_t *borBucketHeapNew(void)
{
    bor_bucketheap_t *bh;
    size_t segm_size = pagesize();

    bh = BOR_ALLOC(bor_bucketheap_t);
    bh->bucket = borSegmArrNew(sizeof(bor_list_t), segm_size);
    bh->bucket_size = 0;
    bh->node_size = 0;
    bh->lowest_key = INT_MAX;

    return bh;
}


void borBucketHeapDel(bor_bucketheap_t *bh)
{
    if (bh->bucket)
        borSegmArrDel(bh->bucket);
    BOR_FREE(bh);
}

void borBucketHeapAdd(bor_bucketheap_t *bh, int key,
                      bor_bucketheap_node_t *n)
{
    int i;
    bor_list_t *bucket;

    if ((unsigned)key > BOR_BUCKETHEAP_EXCESSIVELY_HIGH_KEY){
        fprintf(stderr, "BucketHeap Error: The key %d is excessively high"
                        " and the bucket-heap won't\n"
                        "accept it. It would require at least %lu bytes to"
                        " allocate (probably more).\n",
                        key, sizeof(bor_list_t) * key);
        exit(-1);
    }

    if (key >= bh->bucket_size){
        for (i = bh->bucket_size; i <= key; ++i){
            bucket = borSegmArrGet(bh->bucket, i);
            borListInit(bucket);
        }

        bh->bucket_size = key + 1;
    }

    bucket = borSegmArrGet(bh->bucket, key);
    borListAppend(bucket, n);
    ++bh->node_size;
    if (key < bh->lowest_key)
        bh->lowest_key = key;
}
