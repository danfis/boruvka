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

#include <Python.h>
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "quat.h"
#include "point_cloud.h"
#include "mat3.h"
#include "mat4.h"

static PyMethodDef fer_methods[] = {
    {NULL}  /* Sentinel */
};

static struct PyModuleDef fermatmodule = {
   PyModuleDef_HEAD_INIT,
   "fermat",   /* name of module */
   "Python bindings of Fermat Library", /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   fer_methods
};

PyMODINIT_FUNC
PyInit_fermat(void) 
{
    PyObject* m;

    m = PyModule_Create(&fermatmodule);

    vec2Init(m);
    vec3Init(m);
    vec4Init(m);
    quatInit(m);
    pcInit(m);
    mat3Init(m);
    mat4Init(m);

    return m;
}
