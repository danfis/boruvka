/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __MG_DBG_H__
#define __MG_DBG_H__

/**
 * Some macros which can be used for printing debug info to stderr if macro
 * NDEBUG not defined.
 *
 * DBG_PROLOGUE can be specified as string and this string will be
 * prepended to output text
 */
#ifndef NDEBUG

#include <stdio.h>

#ifndef DBG_PROLOGUE
# define DBG_PROLOGUE
#endif

#ifndef ERR_PROLOGUE
# define ERR_PROLOGUE "Error: "
#endif

# define DBG(format, ...) do { \
    fprintf(stderr, DBG_PROLOGUE "%s :: " format "\n", __func__, ## __VA_ARGS__); \
    fflush(stderr); \
    } while (0)

# define DBG2(str) do { \
    fprintf(stderr, DBG_PROLOGUE "%s :: " str "\n", __func__); \
    fflush(stderr); \
    } while (0)

# define DBG_VEC3(vec, prefix) do {\
    fprintf(stderr, DBG_PROLOGUE "%s :: %s[" MG_REAL_STR " " MG_REAL_STR " " MG_REAL_STR "]\n", \
            __func__, prefix, mgVec3X(vec), \
                              mgVec3Y(vec), \
                              mgVec3Z(vec)); \
    fflush(stderr); \
    } while (0)

# define ERR(format, ...) do { \
    fprintf(stderr, ERR_PROLOGUE format "\n", ## __VA_ARGS__); \
    fflush(stderr); \
    } while (0)

# define ERR2(str) do { \
    fprintf(stderr, ERR_PROLOGUE str "\n"); \
    fflush(stderr); \
    } while (0)

#else
# define DBG(format, ...)
# define DBG2(str)
# define DBG_VEC3(v, prefix)
# define ERR(format, ...)
# define ERR2(str)
#endif

#endif /* __MG_DBG_H__ */
