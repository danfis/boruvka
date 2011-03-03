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

#ifndef PY_QUAT_H_
#define PY_QUAT_H_

#include <Python.h>
#include <fermat/quat.h>

struct _py_quat {
    PyObject_HEAD
    fer_quat_t v;
};
typedef struct _py_quat py_quat;

extern PyTypeObject py_quat_type;

void quatInit(PyObject *module);

#endif /* PY_QUAT_H_ */
