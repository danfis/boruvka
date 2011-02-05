#include "common.h"

void __ferDefaultDealloc(PyObject *self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}
