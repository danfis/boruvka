#include <Python.h>
#include "vec3.h"
#include "vec4.h"
#include "quat.h"
#include "point_cloud.h"

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

    vec3Init(m);
    vec4Init(m);
    quatInit(m);
    pcInit(m);

    return m;
}
