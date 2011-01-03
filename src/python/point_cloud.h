#ifndef PY_PC_H_
#define PY_PC_H_

#include <Python.h>
#include <fermat/point_cloud.h>

struct _py_pc {
    PyObject_HEAD
    fer_pc_t *pc;
};
typedef struct _py_pc py_pc;

extern PyTypeObject py_pc_type;

void pcInit(PyObject *module);

#endif /* PY_PC_H_ */
