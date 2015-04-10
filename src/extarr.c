#include <stdio.h>
#include <unistd.h>
#include <boruvka/alloc.h>
#include <boruvka/extarr.h>

bor_extarr_t *_borExtArrNew(size_t el_size, size_t segment_size,
                            bor_extarr_el_init_fn init_fn,
                            const void *init_data)
{
    bor_extarr_t *arr;

    arr = BOR_ALLOC(bor_extarr_t);
    arr->arr = borSegmArrNew(el_size, segment_size);
    if (arr->arr == NULL){
        fprintf(stderr, "Error: Cannot create a segmented array with elemen"
                        " size of %d bytes and segment size of %d bytes.\n",
                        (int)el_size, (int)segment_size);
        BOR_FREE(arr);
        exit(-1);
    }

    arr->size = 0;
    arr->init_fn = NULL;
    arr->init_data = NULL;

    if (init_fn){
        arr->init_fn   = init_fn;
        arr->init_data = (void *)init_data;
    }else if (init_data){
        arr->init_data = BOR_ALLOC_ARR(char, el_size);
        memcpy(arr->init_data, init_data, el_size);
    }

    return arr;
}

bor_extarr_t *borExtArrNew2(size_t el_size,
                            size_t init_pagesize_multiple,
                            size_t min_els_per_segment,
                            bor_extarr_el_init_fn init_fn,
                            const void *init_data)
{
    size_t segment_size;

    // compute best segment size
    segment_size = sysconf(_SC_PAGESIZE);
    segment_size *= init_pagesize_multiple;
    while (segment_size < min_els_per_segment * el_size)
        segment_size *= 2;

    return _borExtArrNew(el_size, segment_size, init_fn, init_data);
}

bor_extarr_t *borExtArrNew(size_t el_size,
                           bor_extarr_el_init_fn init_fn,
                           const void *init_data)
{
    return borExtArrNew2(el_size, BOR_EXTARR_PAGESIZE_MULTIPLE,
                         BOR_EXTARR_MIN_ELS_PER_SEGMENT,
                         init_fn, init_data);
}

void borExtArrDel(bor_extarr_t *arr)
{
    if (arr->arr)
        borSegmArrDel(arr->arr);
    if (!arr->init_fn && arr->init_data)
        BOR_FREE(arr->init_data);
    BOR_FREE(arr);
}

bor_extarr_t *borExtArrClone(const bor_extarr_t *src)
{
    bor_extarr_t *arr;
    size_t elsize, segmsize;
    void *data;
    size_t i;

    elsize = src->arr->el_size;
    segmsize = src->arr->segm_size;

    arr = _borExtArrNew(elsize, segmsize, src->init_fn, src->init_data);
    arr->size = src->size;
    for (i = 0; i < src->size; ++i){
        data = borSegmArrGet(arr->arr, i);
        memcpy(data, borSegmArrGet(src->arr, i), elsize);
    }

    return arr;
}

void borExtArrResize(bor_extarr_t *arr, size_t eli)
{
    size_t i;
    void *data;

    if (!arr->init_fn && !arr->init_data){
        arr->size = eli + 1;
        return;
    }

    for (i = arr->size; i < eli + 1; ++i){
        data = borSegmArrGet(arr->arr, i);
        if (arr->init_fn){
            arr->init_fn(data, i, arr->init_data);
        }else{
            memcpy(data, arr->init_data, arr->arr->el_size);
        }
    }

    arr->size = eli + 1;
}
