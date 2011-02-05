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

#ifndef PY_PC_H_
#define PY_PC_H_

#include <Python.h>
#include <fermat/pc.h>

struct _py_pc {
    PyObject_HEAD
    fer_pc_t *pc;
};
typedef struct _py_pc py_pc;

extern PyTypeObject py_pc_type;

void pcInit(PyObject *module);

#endif /* PY_PC_H_ */
