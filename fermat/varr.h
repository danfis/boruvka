/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_VARR_H__
#define __FER_VARR_H__

#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _FER_VARR_STRUCT(struct_prefix) \
    struct_prefix##_varr_t
#define _FER_VARR_STRUCT2(struct_prefix) \
    _##struct_prefix##_varr_t

#define _FER_VARR_DECL_STRUCT(type, struct_prefix) \
    struct _FER_VARR_STRUCT2(struct_prefix) { \
        type *arr; \
        size_t len; \
        size_t alloc; \
    }; \
    typedef struct _FER_VARR_STRUCT2(struct_prefix) _FER_VARR_STRUCT(struct_prefix)

#define _FER_VARR_FUNC(func_prefix, name) \
    func_prefix ## name

#define _FER_VARR_DECL_INIT(type, sprefix, fprefix) \
    _fer_inline void _FER_VARR_FUNC(fprefix, Init) \
                        (_FER_VARR_STRUCT(sprefix) *arr, \
                         size_t init_size)

/**
 * Variable Length Arrays
 * =======================
 */
struct _fer_varr_t {
    void *arr;
    size_t len;
    size_t alloc;
};
typedef struct _fer_varr_t fer_varr_t;

/**
 * Initializes array, {init_size} is initial number of elements and
 * {elemsize} is size (in bytes) of a single element in array.
 */
void ferVArrInit(fer_varr_t *arr, size_t init_size, size_t elemsize);

/**
 * Free all allocated memory
 */
void ferVArrDestroy(fer_varr_t *arr);

/**
 * Expands array, factor-times more memory will be allocated.
 * Note, that "realloc" is used, so the elements can change place in the
 * memory.
 */
void ferVArrExpand(fer_varr_t *arr, size_t factor, size_t elemsize);


/**
 * Declare your own struct with typed array elements.
 * ~~~~
 * struct _{struct_name} {
 *     {type} *arr;
 *     size_t len;
 *     size_t alloc;
 * };
 * typedef struct _{struct_name) {struct_name};
 */
#define FER_VARR_STRUCT(type, struct_name) \
    struct _##struct_name { \
        type *arr; \
        size_t len; \
        size_t alloc; \
    }; \
    typedef struct _##struct_name struct_name;

/**
 * Define typed init (inline) function.
 * ~~~~
 * _fer_inline {type} {func_name}({struct_name} *arr, size_t init_size);
 */
#define FER_VARR_INIT(struct_name, func_name) \
    _fer_inline void func_name(struct_name *arr, size_t init_size) \
    { \
        ferVArrInit((fer_varr_t *)arr, init_size, sizeof(*(arr)->arr)); \
    }

/**
 * Define typed destroy (inline) function.
 * ~~~~
 * _fer_inline {type} {func_name}({struct_name} *arr);
 */
#define FER_VARR_DESTROY(struct_name, func_name) \
    _fer_inline void func_name(struct_name *arr) \
    { \
        ferVArrDestroy((fer_varr_t *)arr); \
    }

/**
 * Define typed get (inline) function.
 * ~~~~
 * _fer_inline {type} {func_name}(const {struct_name} *arr, size_t i);
 */
#define FER_VARR_GET(type, struct_name, func_name) \
    _fer_inline type func_name(const struct_name *arr, size_t i) \
    { \
        return arr->arr[i]; \
    }

/**
 * Define typed set (inline) function.
 * ~~~~
 * _fer_inline void {func_name}({struct_name} *arr, size_t i, {type} val);
 */
#define FER_VARR_SET(type, struct_name, func_name) \
    _fer_inline void func_name(const struct_name *arr, size_t i, type val) \
    { \
        arr->arr[i] = val; \
    }

/**
 * Defined typed add (inline) function with predefined expansion factor 2.
 * The element is added at the end of the array and index of the element in
 * array is returned.
 * ~~~~
 * _fer_inline siez_t {func_name}({struct_name} *arr, {type} val);
 */
#define FER_VARR_ADD(type, struct_name, func_name) \
    _fer_inline size_t func_name(struct_name *arr, type val) \
    { \
        if (arr->len == arr->alloc) \
            ferVArrExpand((fer_varr_t *)arr, 2, sizeof(type)); \
        arr->arr[arr->len] = val; \
        return arr->len++; \
    }


/**
 * Define typed remove (inline) function that removes specified element
 * from array, so that last element is moved to this position and the array
 * is shrinked.
 * ~~~~
 * _fer_inline void {func_name}({struct_name) *arr, size_t i);
 */
#define FER_VARR_REMOVE(struct_name, func_name) \
    _fer_inline void func_name(struct_name *arr, size_t i) \
    { \
        arr->arr[i] = arr->arr[--arr->len]; \
    }

/**
 * Makes declaration of the struct with specified name.
 * Makes definition of init, destroy, get, set, add, and remove
 * functions with specified prefix as ({prefix}Init, {prefix}Destroy, ...).
 */
#define FER_VARR_DECL(type, struct_name, func_prefix) \
    FER_VARR_STRUCT(type, struct_name) \
    FER_VARR_INIT(struct_name, func_prefix ## Init) \
    FER_VARR_DESTROY(struct_name, func_prefix ## Destroy) \
    FER_VARR_GET(type, struct_name, func_prefix ## Get) \
    FER_VARR_SET(type, struct_name, func_prefix ## Set) \
    FER_VARR_ADD(type, struct_name, func_prefix ## Add) \
    FER_VARR_REMOVE(struct_name, func_prefix ## Remove)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_VARR_H__ */
