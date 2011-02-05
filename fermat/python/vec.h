/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef PY_VEC_H_
#define PY_VEC_H_

#include <Python.h>
#include <fermat/vec.h>

struct _py_vec {
    PyObject_HEAD
    fer_vec_t v;
};
typedef struct _py_vec py_vec;

extern PyTypeObject py_vec_type;

void vecInit(PyObject *module);

#endif /* PY_VEC_H_ */
