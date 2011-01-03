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
