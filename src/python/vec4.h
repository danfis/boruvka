#ifndef PY_VEC4_H_
#define PY_VEC4_H_

#include <Python.h>
#include <mg/vec4.h>

struct _py_vec4 {
    PyObject_HEAD
    mg_vec4_t v;
};
typedef struct _py_vec4 py_vec4;

extern PyTypeObject py_vec4_type;

void vec4Init(PyObject *module);

#endif /* PY_VEC4_H_ */
