#ifndef __BOR_DATAARR_H__
#define __BOR_DATAARR_H__

#include <boruvka/segmarr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Extendable Array
 * =================
 *
 * Extendable array is a wrapper around segmented array (see bor_segmarr_t)
 * which provides counting of the elements stored in the array and their
 * automatic initialization through template data or initialization
 * function. The array is extendable but never changes the place of once
 * stored data, so the pointers to the array are always valid.
 *
 * See bor_extarr_t.
 */

/**
 * Default minimal number of elements per segment.
 */
#define BOR_EXTARR_MIN_ELS_PER_SEGMENT 32

/**
 * Default size of a segment expressed as a multiple of a pagesize.
 */
#define BOR_EXTARR_PAGESIZE_MULTIPLE 8

/**
 * Callback for element initialization.
 * The function gets pointer to the element stored in the array, index of
 * that element and optional user data.
 */
typedef void (*bor_extarr_el_init_fn)(void *el, int idx, const void *userdata);

struct _bor_extarr_t {
    bor_segmarr_t *arr; /*!< Underlying segmented array */
    size_t size;        /*!< Number of elements stored in the array. */

    /*!< Initialization structures */
    bor_extarr_el_init_fn init_fn;
    void *init_data;
};
typedef struct _bor_extarr_t bor_extarr_t;

/**
 * Creates a new extendable array.
 * The size of one segment is determined automatically as multiple of
 * pagesize.
 *
 * If init_fn function callback is non-NULL, it is used for initialization
 * of each element before the element is first returned. In this case
 * init_data is used as last argument of init_fn function.
 * If init_fn is non-NULL, the init_data should point to a memory of el_size
 * size and the data it points at are copied to the internal storage and used
 * for the initialization of each element before it is first returned.
 */
bor_extarr_t *borExtArrNew(size_t el_size,
                           bor_extarr_el_init_fn init_fn,
                           const void *init_data);

/**
 * Same as borExtArrNew() but initial multiple of pagesize can be provided
 * as well as minimal number of elements per one segment.
 * For example if we want to have one segment of the array to have at least
 * 128 times pagesize bytes, we set init_pagesize_multiple to 128.
 */
bor_extarr_t *borExtArrNew2(size_t el_size,
                            size_t init_pagesize_multiple,
                            size_t min_els_per_segment,
                            bor_extarr_el_init_fn init_fn,
                            const void *init_data);

/**
 * Deletes extendable array
 */
void borExtArrDel(bor_extarr_t *arr);

/**
 * Creates an exact copy of the extendable array.
 */
bor_extarr_t *borExtArrClone(const bor_extarr_t *arr);

/**
 * Returns pointer to the i'th element of the array.
 * If i is greater than the current size of array, the array is
 * automatically extended.
 */
_bor_inline void *borExtArrGet(bor_extarr_t *arr, size_t i);

/**
 * Returns number of elements stored in the array.
 */
_bor_inline size_t borExtArrSize(const bor_extarr_t *arr);

/**
 * Ensures that the array has at least i elements.
 */
void borExtArrResize(bor_extarr_t *arr, size_t i);

/**** INLINES ****/
_bor_inline void *borExtArrGet(bor_extarr_t *arr, size_t i)
{
    if (i >= arr->size){
        borExtArrResize(arr, i);
    }

    return borSegmArrGet(arr->arr, i);
}

_bor_inline size_t borExtArrSize(const bor_extarr_t *arr)
{
    return arr->size;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_DATAARR_H__ */
