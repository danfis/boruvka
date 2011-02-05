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

#ifndef PY_VEC2_H_
#define PY_VEC2_H_

#include <Python.h>
#include <fermat/vec2.h>

struct _py_vec2 {
    PyObject_HEAD
    fer_vec2_t v;
};
typedef struct _py_vec2 py_vec2;

extern PyTypeObject py_vec2_type;

void vec2Init(PyObject *module);

#endif /* PY_VEC2_H_ */
