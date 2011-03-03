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

#ifndef PY_MAT4_H_
#define PY_MAT4_H_

#include <Python.h>
#include <fermat/mat4.h>

struct _py_mat4 {
    PyObject_HEAD
    fer_mat4_t m;
};
typedef struct _py_mat4 py_mat4;

extern PyTypeObject py_mat4_type;

void mat4Init(PyObject *module);

#endif /* PY_MAT4_H_ */

