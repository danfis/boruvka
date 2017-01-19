/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_VARR_H__
#define __BOR_VARR_H__

#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _BOR_VARR_STRUCT(struct_prefix) \
    struct_prefix##_varr_t
#define _BOR_VARR_STRUCT2(struct_prefix) \
    _##struct_prefix##_varr_t

#define _BOR_VARR_DECL_STRUCT(type, struct_prefix) \
    struct _BOR_VARR_STRUCT2(struct_prefix) { \
        type *arr; \
        size_t size; \
        size_t alloc; \
    }; \
    typedef struct _BOR_VARR_STRUCT2(struct_prefix) _BOR_VARR_STRUCT(struct_prefix)

#define _BOR_VARR_FUNC(func_prefix, name) \
    func_prefix ## name

#define _BOR_VARR_DECL_INIT(type, sprefix, fprefix) \
    _bor_inline void _BOR_VARR_FUNC(fprefix, Init) \
                        (_BOR_VARR_STRUCT(sprefix) *arr, \
                         size_t init_size)

/**
 * Variable Length Arrays
 * =======================
 */
struct _bor_varr_t {
    void *arr;
    size_t size;
    size_t alloc;
};
typedef struct _bor_varr_t bor_varr_t;

/**
 * Initializes array, {init_size} is initial number of elements and
 * {elemsize} is size (in bytes) of a single element in array.
 */
void borVArrInit(bor_varr_t *arr, size_t init_size, size_t elemsize);

/**
 * Free all allocated memory
 */
void borVArrDestroy(bor_varr_t *arr);

/**
 * Expands array, factor-times more memory will be allocated.
 * Note, that "realloc" is used, so the elements can change place in the
 * memory.
 */
void borVArrExpand(bor_varr_t *arr, size_t factor, size_t elemsize);


/**
 * Declare your own struct with typed array elements.
 * ~~~~
 * struct _{struct_name} {
 *     {type} *arr;
 *     size_t size;
 *     size_t alloc;
 * };
 * typedef struct _{struct_name) {struct_name};
 */
#define BOR_VARR_STRUCT(type, struct_name) \
    struct _##struct_name { \
        type *arr; \
        size_t size; \
        size_t alloc; \
    }; \
    typedef struct _##struct_name struct_name;

/**
 * Define typed init (inline) function.
 * ~~~~
 * _bor_inline {type} {func_name}({struct_name} *arr, size_t init_size);
 */
#define BOR_VARR_INIT(struct_name, func_name) \
    _bor_inline void func_name(struct_name *arr, size_t init_size) \
    { \
        borVArrInit((bor_varr_t *)arr, init_size, sizeof(*(arr)->arr)); \
    }

/**
 * Define typed destroy (inline) function.
 * ~~~~
 * _bor_inline {type} {func_name}({struct_name} *arr);
 */
#define BOR_VARR_DESTROY(struct_name, func_name) \
    _bor_inline void func_name(struct_name *arr) \
    { \
        borVArrDestroy((bor_varr_t *)arr); \
    }

/**
 * Define typed get (inline) function.
 * ~~~~
 * _bor_inline {type} {func_name}(const {struct_name} *arr, size_t i);
 */
#define BOR_VARR_GET(type, struct_name, func_name) \
    _bor_inline type func_name(const struct_name *arr, size_t i) \
    { \
        return arr->arr[i]; \
    }

/**
 * Define typed set (inline) function.
 * ~~~~
 * _bor_inline void {func_name}({struct_name} *arr, size_t i, {type} val);
 */
#define BOR_VARR_SET(type, struct_name, func_name) \
    _bor_inline void func_name(const struct_name *arr, size_t i, type val) \
    { \
        arr->arr[i] = val; \
    }

/**
 * Defined typed add (inline) function with predefined expansion factor 2.
 * The element is added at the end of the array and index of the element in
 * array is returned.
 * ~~~~
 * _bor_inline siez_t {func_name}({struct_name} *arr, {type} val);
 */
#define BOR_VARR_ADD(type, struct_name, func_name) \
    _bor_inline size_t func_name(struct_name *arr, type val) \
    { \
        if (arr->size == arr->alloc) \
            borVArrExpand((bor_varr_t *)arr, 2, sizeof(type)); \
        arr->arr[arr->size] = val; \
        return arr->size++; \
    }


/**
 * Define typed remove (inline) function that removes specified element
 * from array, so that last element is moved to this position and the array
 * is shrinked.
 * ~~~~
 * _bor_inline void {func_name}({struct_name) *arr, size_t i);
 */
#define BOR_VARR_REMOVE(struct_name, func_name) \
    _bor_inline void func_name(struct_name *arr, size_t i) \
    { \
        arr->arr[i] = arr->arr[--arr->size]; \
    }

/**
 * Makes declaration of the struct with specified name.
 * Makes definition of init, destroy, get, set, add, and remove
 * functions with specified prefix as ({prefix}Init, {prefix}Destroy, ...).
 */
#define BOR_VARR_DECL(type, struct_name, func_prefix) \
    BOR_VARR_STRUCT(type, struct_name) \
    BOR_VARR_INIT(struct_name, func_prefix ## Init) \
    BOR_VARR_DESTROY(struct_name, func_prefix ## Destroy) \
    BOR_VARR_DESTROY(struct_name, func_prefix ## Free) \
    BOR_VARR_GET(type, struct_name, func_prefix ## Get) \
    BOR_VARR_SET(type, struct_name, func_prefix ## Set) \
    BOR_VARR_ADD(type, struct_name, func_prefix ## Add) \
    BOR_VARR_REMOVE(struct_name, func_prefix ## Remove)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_VARR_H__ */
