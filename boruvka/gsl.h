/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_GSL_H__
#define __BOR_GSL_H__

#include <boruvka/core.h>

#ifndef BOR_GSL
# error "Boruvka is not compiled with GSL support!"
#endif /* BOR_GSL */

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>


#ifdef BOR_SINGLE
# define bor_gsl_vector gsl_vector_float
# define bor_gsl_vector_alloc gsl_vector_float_alloc
# define bor_gsl_vector_calloc gsl_vector_float_calloc
# define bor_gsl_vector_free gsl_vector_float_free
# define bor_gsl_vector_get gsl_vector_float_get
# define bor_gsl_vector_set gsl_vector_float_set
# define bor_gsl_vector_set_all gsl_vector_float_set_all
# define bor_gsl_vector_set_zero gsl_vector_float_set_zero
# define bor_gsl_vector_set_basis gsl_vector_float_set_basis
# define bor_gsl_vector_add gsl_vector_float_add
# define bor_gsl_vector_sub gsl_vector_float_sub
# define bor_gsl_vector_mul gsl_vector_float_mul
# define bor_gsl_vector_div gsl_vector_float_div
# define bor_gsl_vector_scale gsl_vector_float_scale
# define bor_gsl_vector_add_constant gsl_vector_float_add_consant
# define bor_gsl_vector_memcpy gsl_vector_float_memcpy

# define bor_gsl_vector_view gsl_vector_float_view
# define bor_gsl_vector_view_array gsl_vector_float_view_array
# define bor_gsl_vector_subvector gsl_vector_float_subvector

# define bor_gsl_matrix gsl_matrix_float
# define bor_gsl_matrix_alloc gsl_matrix_float_alloc
# define bor_gsl_matrix_calloc gsl_matrix_float_calloc
# define bor_gsl_matrix_free gsl_matrix_float_free
# define bor_gsl_matrix_get gsl_matrix_float_get
# define bor_gsl_matrix_set gsl_matrix_float_set
# define bor_gsl_matrix_set_all gsl_matrix_float_set_all
# define bor_gsl_matrix_set_zero gsl_matrix_float_set_zero
# define bor_gsl_matrix_set_identity gsl_matrix_float_set_identity
# define bor_gsl_matrix_add gsl_matrix_float_add
# define bor_gsl_matrix_sub gsl_matrix_float_sub
# define bor_gsl_matrix_mul_elements gsl_matrix_float_mul_elements
# define bor_gsl_matrix_div_elements gsl_matrix_float_div_elements
# define bor_gsl_matrix_scale gsl_matrix_float_scale
# define bor_gsl_matrix_add_constant gsl_matrix_float_add_constant
# define bor_gsl_matrix_memcpy gsl_matrix_float_memcpy

# define bor_gsl_matrix_view gsl_matrix_float_view
# define bor_gsl_matrix_const_view gsl_matrix_float_const_view
# define bor_gsl_matrix_view_array gsl_matrix_float_view_array
# define bor_gsl_matrix_submatrix gsl_matrix_float_submatrix
# define bor_gsl_matrix_const_submatrix gsl_matrix_float_const_submatrix
# define bor_gsl_matrix_row gsl_matrix_float_row
# define bor_gsl_matrix_column gsl_matrix_float_column

# define bor_gsl_blas_gemm gsl_blas_sgemm
# define bor_gsl_blas_gemv gsl_blas_sgemv

#else /* BOR_SINGLE */
# define bor_gsl_vector gsl_vector
# define bor_gsl_vector_alloc gsl_vector_alloc
# define bor_gsl_vector_calloc gsl_vector_calloc
# define bor_gsl_vector_free gsl_vector_free
# define bor_gsl_vector_get gsl_vector_get
# define bor_gsl_vector_set gsl_vector_set
# define bor_gsl_vector_set_all gsl_vector_set_all
# define bor_gsl_vector_set_zero gsl_vector_set_zero
# define bor_gsl_vector_set_basis gsl_vector_set_basis
# define bor_gsl_vector_add gsl_vector_add
# define bor_gsl_vector_sub gsl_vector_sub
# define bor_gsl_vector_mul gsl_vector_mul
# define bor_gsl_vector_div gsl_vector_div
# define bor_gsl_vector_scale gsl_vector_scale
# define bor_gsl_vector_add_constant gsl_vector_add_consant
# define bor_gsl_vector_memcpy gsl_vector_memcpy

# define bor_gsl_vector_view gsl_vector_view
# define bor_gsl_vector_view_array gsl_vector_view_array
# define bor_gsl_vector_subvector gsl_vector_subvector

# define bor_gsl_matrix gsl_matrix
# define bor_gsl_matrix_alloc gsl_matrix_alloc
# define bor_gsl_matrix_calloc gsl_matrix_calloc
# define bor_gsl_matrix_free gsl_matrix_free
# define bor_gsl_matrix_get gsl_matrix_get
# define bor_gsl_matrix_set gsl_matrix_set
# define bor_gsl_matrix_set_all gsl_matrix_set_all
# define bor_gsl_matrix_set_zero gsl_matrix_set_zero
# define bor_gsl_matrix_set_identity gsl_matrix_set_identity
# define bor_gsl_matrix_add gsl_matrix_add
# define bor_gsl_matrix_sub gsl_matrix_sub
# define bor_gsl_matrix_mul_elements gsl_matrix_mul_elements
# define bor_gsl_matrix_div_elements gsl_matrix_div_elements
# define bor_gsl_matrix_scale gsl_matrix_scale
# define bor_gsl_matrix_add_constant gsl_matrix_add_constant
# define bor_gsl_matrix_memcpy gsl_matrix_memcpy

# define bor_gsl_matrix_view gsl_matrix_view
# define bor_gsl_matrix_const_view gsl_matrix_const_view
# define bor_gsl_matrix_view_array gsl_matrix_view_array
# define bor_gsl_matrix_submatrix gsl_matrix_submatrix
# define bor_gsl_matrix_const_submatrix gsl_matrix_const_submatrix
# define bor_gsl_matrix_row gsl_matrix_row
# define bor_gsl_matrix_column gsl_matrix_column

# define bor_gsl_blas_gemm gsl_blas_dgemm
# define bor_gsl_blas_gemv gsl_blas_dgemv

#endif /* BOR_SINGLE */

#endif /* __BOR_GSL_H__ */
