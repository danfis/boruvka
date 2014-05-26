#include "boruvka/segmarr.h"
#include "boruvka/alloc.h"

bor_segmarr_t *borSegmArrNew(size_t el_size, size_t els_per_segm)
{
    bor_segmarr_t *arr;

    arr = BOR_ALLOC(bor_segmarr_t);
    arr->el_size    = el_size;
    arr->segm_size  = els_per_segm;
    arr->segm       = NULL;
    arr->num_segm   = 0;
    arr->alloc_segm = 0;

    return arr;
}

void borSegmArrDel(bor_segmarr_t *arr)
{
    size_t i;

    if (arr->segm){
        for (i = 0; i < arr->num_segm; ++i)
            BOR_FREE(arr->segm[i]);
        BOR_FREE(arr->segm);
    }
    BOR_FREE(arr);
}

void borSegmArrExpandSegments(bor_segmarr_t *arr, size_t num_segs)
{
    size_t i;

    if (num_segs <= arr->num_segm)
        return;

    if (arr->alloc_segm < num_segs){
        arr->alloc_segm *= 2;
        if (arr->alloc_segm < num_segs)
            arr->alloc_segm = num_segs;

        arr->segm = BOR_REALLOC_ARR(arr->segm, char *, arr->alloc_segm);

        // zeroize yet not needed segment slots
        for (i = num_segs; i < arr->alloc_segm; ++i){
            arr->segm[i] = NULL;
        }
    }

    // allocate all needed segments
    for (; arr->num_segm < num_segs; ++arr->num_segm){
        arr->segm[arr->num_segm] = BOR_ALLOC_ARR(char, arr->el_size * arr->segm_size);
    }
}
