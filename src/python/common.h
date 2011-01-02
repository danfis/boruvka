#ifndef PY_COMMON_H_
#define PY_COMMON_H_

#include <Python.h>
#include <mg/core.h>

_mg_inline mg_real_t numberAsReal(PyObject *n)
{
    PyObject *onum;
    mg_real_t r;

    if (PyFloat_Check(n))
        return PyFloat_AsDouble(n);

    onum = PyNumber_Float(n);
    r = PyFloat_AsDouble(onum);
    Py_DECREF(onum);

    return r;
}

#define CHECK_VEC3(o) \
    if (!PyObject_TypeCheck((o), &py_vec3_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected Vec3"); \
        return NULL; \
    }

#define CHECK_VEC4(o) \
    if (!PyObject_TypeCheck((o), &py_vec4_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected Vec4"); \
        return NULL; \
    }

#define CHECK_QUAT(o) \
    if (!PyObject_TypeCheck((o), &py_quat_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected Quat"); \
        return NULL; \
    }

#define CHECK_FLOAT2(o, ret) \
    if (!PyNumber_Check(o)){ \
        PyErr_SetString(PyExc_TypeError, "Expected float"); \
        return (ret); \
    }
#define CHECK_FLOAT(o) CHECK_FLOAT2(o, NULL)


#endif /* PY_COMMON_H_ */
