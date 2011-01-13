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

#ifndef PY_MAT3_H_
#define PY_MAT3_H_

#include <Python.h>
#include <fermat/mat3.h>

struct _py_mat3 {
    PyObject_HEAD
    fer_mat3_t m;
};
typedef struct _py_mat3 py_mat3;

extern PyTypeObject py_mat3_type;

void mat3Init(PyObject *module);

#endif /* PY_MAT3_H_ */

