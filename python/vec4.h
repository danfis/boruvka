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

#ifndef PY_VEC4_H_
#define PY_VEC4_H_

#include <Python.h>
#include <fermat/vec4.h>

struct _py_vec4 {
    PyObject_HEAD
    fer_vec4_t v;
};
typedef struct _py_vec4 py_vec4;

extern PyTypeObject py_vec4_type;

void vec4Init(PyObject *module);

#endif /* PY_VEC4_H_ */
