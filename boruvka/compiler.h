/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_COMPILER_H__
#define __FER_COMPILER_H__

#include <stddef.h>

/**
 * Compiler - Compiler Specific Macros
 * ====================================
 */


/** vvvv */

/**
 * Returns offset of member in given type (struct).
 */
#define fer_offsetof(TYPE, MEMBER) offsetof(TYPE, MEMBER)
/*#define fer_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)*/

/**
 * Returns container of given member
 */
#define fer_container_of(ptr, type, member) \
    (type *)( (char *)ptr - fer_offsetof(type, member))


/**
 * Marks inline function.
 */
#ifdef __GNUC__
#  ifdef FER_DEBUG
#    define _fer_inline static
#  else /* FER_DEBUG */
#    ifdef __NO_INLINE__
#      define _fer_inline static
#    else /* __NO_INLINE */
#      define _fer_inline static inline __attribute__((always_inline))
#    endif /* __NO_INLINE */
#  endif /* FER_DEBUG */
#else /* __GNUC__ */
# define _fer_inline static inline
#endif /* __GNUC__ */


/**
 * __prefetch(x)  - prefetches the cacheline at "x" for read
 * __prefetchw(x) - prefetches the cacheline at "x" for write
 */
#ifdef __GNUC__
# define _fer_prefetch(x) __builtin_prefetch(x)
# define _fer_prefetchw(x) __builtin_prefetch(x,1)
#else /* __GNUC__ */
# define _fer_prefetch(x)
# define _fer_prefetchw(x)
#endif /* __GNUC__ */

/**
 * Using this macros you can specify is it's likely or unlikely that branch
 * will be used.
 * Comes from linux header file ./include/compiler.h
 */
#ifdef __GNUC__
# define fer_likely(x) __builtin_expect(!!(x), 1)
# define fer_unlikely(x) __builtin_expect(!!(x), 0)
#else /* __GNUC__ */
# define fer_likely(x) !!(x)
# define fer_unlikely(x) !!(x)
#endif /* __GNUC__ */

#ifdef __GNUC__
# define fer_aligned(x) __attribute__ ((aligned(x)))
# define fer_packed __attribute__ ((packed))
#else /* __GNUC__ */
# define fer_aligned(x)
# define fer_packed
#endif /* __GNUC__ */


#ifdef __ICC
/* disable unused parameter warning */
# pragma warning(disable:869)
/* disable annoying "operands are evaluated in unspecified order" warning */
# pragma warning(disable:981)
#endif /* __ICC */

/** ^^^^ */

#endif /* __FER_COMPILER_H__ */
