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

#include "common.h"
#include "vec.h"

static int vecObjInit(py_vec *self, PyObject *args, PyObject *kwds);
static void vecDealloc(py_vec* self);
static PyObject *vecAsStr(py_vec *self);

static PyObject *vecCopy(py_vec *self);
static PyObject *vecLen2(py_vec *self);
static PyObject *vecLen(py_vec *self);
static PyObject *vecDist2(py_vec *self, py_vec *o);
static PyObject *vecDist(py_vec *self, py_vec *o);

/** Number protocol functions */
static PyObject *vecAdd(py_vec *self, PyObject *o);
static PyObject *vecSub(py_vec *self, PyObject *o);
static PyObject *vecMul(py_vec *self, PyObject *o);
static PyObject *vecNeg(py_vec *self);
static PyObject *vecDiv(py_vec *self, PyObject *o);
static PyObject *vecAddIn(py_vec *self, PyObject *o);
static PyObject *vecSubIn(py_vec *self, PyObject *o);
static PyObject *vecMulIn(py_vec *self, PyObject *o);
static PyObject *vecDivIn(py_vec *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t vecSeqSize(py_vec *self);
static PyObject *vecSeqGet(py_vec *self, Py_ssize_t i);
static int vecSeqSet(py_vec *self, Py_ssize_t i, PyObject *val);

static PyMethodDef py_vec_methods[] = {
    { "copy", (PyCFunction)vecCopy, METH_NOARGS,
      "Returns deep copy of Vec." },
    { "len2", (PyCFunction)vecLen2, METH_NOARGS,
      "len2() -> float\n"
      "Returns squared length of vector" },
    { "len", (PyCFunction)vecLen, METH_NOARGS,
      "len() -> float\n"
      "Returns length of vector" },
    { "dist2", (PyCFunction)vecDist2, METH_O,
      "dist2(v : Vec) -> float\n"
      "Squared distance of two vectors" },
    { "dist", (PyCFunction)vecDist, METH_O,
      "dist(v : Vec) -> float\n"
      "Distance of two vectors" },
    { NULL }
};

static PyNumberMethods py_vec_num = {
    (binaryfunc)vecAdd, /* binaryfunc nb_add */
    (binaryfunc)vecSub, /* binaryfunc nb_subtract */
    (binaryfunc)vecMul, /* binaryfunc nb_multiply */
    0,                   /* binaryfunc nb_remainder */
    0,                   /* binaryfunc nb_divmod */
    0,                   /* ternaryfunc nb_power */
    (unaryfunc)vecNeg,  /* unaryfunc nb_negative */
    0,                   /* unaryfunc nb_positive */
    0,                   /* unaryfunc nb_absolute */
    0,                   /* inquiry nb_bool */
    0,                   /* unaryfunc nb_invert */
    0,                   /* binaryfunc nb_lshift */
    0,                   /* binaryfunc nb_rshift */
    0,                   /* binaryfunc nb_and */
    0,                   /* binaryfunc nb_xor */
    0,                   /* binaryfunc nb_or */
    0,                   /* unaryfunc nb_int */
    0,                   /* void *nb_reserved */
    0,                   /* unaryfunc nb_float */

    (binaryfunc)vecAddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)vecSubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)vecMulIn, /* binaryfunc nb_inplace_multiply */
    0,                     /* binaryfunc nb_inplace_remainder */
    0,                     /* ternaryfunc nb_inplace_power */
    0,                     /* binaryfunc nb_inplace_lshift */
    0,                     /* binaryfunc nb_inplace_rshift */
    0,                     /* binaryfunc nb_inplace_and */
    0,                     /* binaryfunc nb_inplace_xor */
    0,                     /* binaryfunc nb_inplace_or */

    (binaryfunc)vecDiv,   /* binaryfunc nb_floor_divide */
    (binaryfunc)vecDiv,   /* binaryfunc nb_true_divide */
    (binaryfunc)vecDivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)vecDivIn, /* binaryfunc nb_inplace_true_divide */

    0                      /* unaryfunc nb_index */
};

static PySequenceMethods py_vec_seq = {
    (lenfunc)vecSeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)vecSeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)vecSeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_vec_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Vec",             /* tp_name */
    sizeof(py_vec),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)vecDealloc,    /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)vecAsStr,       /* tp_repr */
    &py_vec_num,              /* tp_as_number */
    &py_vec_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)vecAsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "3D vector",               /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_vec_methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vecObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void vecInit(PyObject *module)
{
    py_vec_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_vec_type) < 0)
        return;

    Py_INCREF(&py_vec_type);
    PyModule_AddObject(module, "Vec", (PyObject *)&py_vec_type);
}


static int vecObjInit(py_vec *self, PyObject *_args, PyObject *kwds)
{
    size_t size;
    Py_ssize_t len = 0;
    PyObject *args = NULL, *val;

    if (PySequence_Check(_args)){
        len = PySequence_Size(_args);
        args = PySequence_Fast(_args, "error");
    }

    if (len != 1){
        if (args){
            Py_DECREF(args);
        }
        PyErr_SetString(PyExc_TypeError, "Invalid argument.");
        return -1;
    }

    val = PySequence_Fast_GET_ITEM(args, 0);
    if (!PyNumber_Check(val)){
        if (args){
            Py_DECREF(args);
        }
        PyErr_SetString(PyExc_TypeError, "Invalid argument.");
        return -1;
    }

    size = PyNumber_AsSsize_t(val, NULL);
    ferVecInit(&self->v, size);

    if (args){
        Py_DECREF(args);
    }

    return 0;
}

static void vecDealloc(py_vec* self)
{
    ferVecDestroy(&self->v);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *vecAsStr(py_vec *self)
{
    char str[100];
    snprintf(str, 100, "<Vec (%d)>", (int)ferVecSize(&self->v));
    return PyUnicode_FromString(str);
}


static PyObject *vecCopy(py_vec *self)
{
    py_vec *v = PyObject_New(py_vec, &py_vec_type);
    ferVecInit(&v->v, ferVecSize(&self->v));
    ferVecCopy(&v->v, &self->v);
    return (PyObject *)v;
}

static PyObject *vecLen2(py_vec *self)
{
    return PyFloat_FromDouble(ferVecLen2(&self->v));
}
static PyObject *vecLen(py_vec *self)
{
    return PyFloat_FromDouble(ferVecLen(&self->v));
}

static PyObject *vecDist2(py_vec *self, py_vec *o)
{
    fer_real_t d;

    CHECK_VEC(o)

    d = ferVecDist2(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}
static PyObject *vecDist(py_vec *self, py_vec *o)
{
    fer_real_t d;

    CHECK_VEC(o)

    d = ferVecDist(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}




static PyObject *vecAdd(py_vec *self, PyObject *o)
{
    py_vec *v;
    fer_real_t f;
    PyObject *tmp;

    if (PyNumber_Check((PyObject *)self)){
        tmp = (PyObject *)self;
        self = (py_vec *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_vec_type)){
        v = PyObject_New(py_vec, &py_vec_type);
        ferVecInit(&v->v, ferVecSize(&self->v));
        if (ferVecAdd2(&v->v, &self->v, &((py_vec *)o)->v) != 0){
            Py_DECREF(v);
            PyErr_SetString(PyExc_TypeError, "Vectors must have same size.");
            return NULL;
        }
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec, &py_vec_type);
        ferVecInit(&v->v, ferVecSize(&self->v));
        f = numberAsReal(o);

        ferVecAddConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vecSub(py_vec *self, PyObject *o)
{
    py_vec *v;
    fer_real_t f;

    if (PyNumber_Check((PyObject *)self)){
        PyErr_SetString(PyExc_TypeError, "Invalid operation");
        return NULL;
    }

    if (PyObject_TypeCheck(o, &py_vec_type)){
        v = PyObject_New(py_vec, &py_vec_type);
        ferVecInit(&v->v, ferVecSize(&self->v));
        if (ferVecSub2(&v->v, &self->v, &((py_vec *)o)->v) != 0){
            Py_DECREF(v);
            PyErr_SetString(PyExc_TypeError, "Vectors must have same size.");
            return NULL;
        }
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec, &py_vec_type);
        ferVecInit(&v->v, ferVecSize(&self->v));
        f = numberAsReal(o);

        ferVecSubConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vecMul(py_vec *self, PyObject *o)
{
    py_vec *v;
    fer_real_t num;
    PyObject *tmp;

    if (PyNumber_Check((PyObject *)self)){
        tmp = (PyObject *)self;
        self = (py_vec *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_vec_type)){
        num = ferVecDot(&self->v, &((py_vec *)o)->v);
        return PyFloat_FromDouble(num);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec, &py_vec_type);
        ferVecInit(&v->v, ferVecSize(&self->v));
        num = numberAsReal(o);

        ferVecCopy(&v->v, &self->v);
        ferVecScale(&v->v, num);
        return (PyObject *)v;
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected Vec or float number");
        return NULL;
    }
}

static PyObject *vecNeg(py_vec *self)
{
    py_vec *v;

    v = PyObject_New(py_vec, &py_vec_type);
    ferVecInit(&v->v, ferVecSize(&self->v));
    ferVecCopy(&v->v, &self->v);
    ferVecScale(&v->v, -FER_ONE);
    return (PyObject *)v;
}

static PyObject *vecDiv(py_vec *self, PyObject *o)
{
    py_vec *v;
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec, &py_vec_type);
    ferVecInit(&v->v, ferVecSize(&self->v));
    ferVecCopy(&v->v, &self->v);
    ferVecScale(&v->v, FER_ONE / num);
    return (PyObject *)v;
}

static PyObject *vecAddIn(py_vec *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec_type)){
        ferVecAdd(&self->v, &((py_vec *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVecAddConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vecSubIn(py_vec *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec_type)){
        ferVecSub(&self->v, &((py_vec *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVecSubConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vecMulIn(py_vec *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVecScale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vecDivIn(py_vec *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVecScale(&self->v, FER_ONE / num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


static Py_ssize_t vecSeqSize(py_vec *self)
{
    return ferVecSize(&self->v);
}

static PyObject *vecSeqGet(py_vec *self, Py_ssize_t i)
{
    if (i >= ferVecSize(&self->v)){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferVecGet(&self->v, i));
}

static int vecSeqSet(py_vec *self, Py_ssize_t i, PyObject *val)
{
    if (i >= ferVecSize(&self->v)){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferVecSet(&self->v, i, numberAsReal(val));
    return 0;
}
