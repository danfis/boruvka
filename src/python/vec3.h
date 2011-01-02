#ifndef PY_VEC3_H_
#define PY_VEC3_H_

#include <Python.h>
#include <mg/vec3.h>

struct _py_vec3 {
    PyObject_HEAD
    mg_vec3_t v;
};
typedef struct _py_vec3 py_vec3;

extern PyTypeObject py_vec3_type;

void vec3Init(PyObject *module);

#endif /* PY_VEC3_H_ */
