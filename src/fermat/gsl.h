/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_GSL_H__
#define __FER_GSL_H__

#include <gsl/gsl_matrix.h>

#ifdef FER_SINGLE
# define __FER_GSL_API(method) gsl_matrix_float_ ## method

# define fer_gsl_matrix gsl_matrix_float
#else /* FER_SINGLE */
# define __FER_GSL_MATRIX_PREFIX gsl_matrix_
# define __FER_GSL_API(method) gsl_matrix_ ## method

# define fer_gsl_matrix gsl_matrix
#endif /* FER_SINGLE */

#define fer_gsl_matrix_alloc __FER_GSL_API(alloc)
#define fer_gsl_matrix_free __FER_GSL_API(free)
#define fer_gsl_matrix_memcpy __FER_GSL_API(memcpy)

#define fer_gsl_matrix_get          __FER_GSL_API(get)
#define fer_gsl_matrix_set          __FER_GSL_API(set)
#define fer_gsl_matrix_set_identity __FER_GSL_API(set_identity)
#define fer_gsl_matrix_set_zero     __FER_GSL_API(set_zero)
#define fer_gsl_matrix_set_all      __FER_GSL_API(set_all)
#define fer_gsl_matrix_add          __FER_GSL_API(add)
#define fer_gsl_matrix_sub          __FER_GSL_API(sub)
#define fer_gsl_matrix_scale        __FER_GSL_API(scale)
#define fer_gsl_matrix_add_constant __FER_GSL_API(add_constant)

#endif /* __FER_GSL_H__ */

