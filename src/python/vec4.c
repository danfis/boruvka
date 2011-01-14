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
#include "vec4.h"

static int vec4ObjInit(py_vec4 *self, PyObject *args, PyObject *kwds);
static PyObject *vec4AsStr(py_vec4 *self);

/** Returns x, y, z coordinates */
static PyObject *vec4GetX(py_vec4 *self, void *coord);
static PyObject *vec4GetY(py_vec4 *self, void *coord);
static PyObject *vec4GetZ(py_vec4 *self, void *coord);
static PyObject *vec4GetW(py_vec4 *self, void *coord);
/** Sets x, y, z coordinates */
static int vec4SetX(py_vec4 *self, PyObject *val, void *coord);
static int vec4SetY(py_vec4 *self, PyObject *val, void *coord);
static int vec4SetZ(py_vec4 *self, PyObject *val, void *coord);
static int vec4SetW(py_vec4 *self, PyObject *val, void *coord);
/** Compare function */
static PyObject *vec4Cmp(PyObject *a, PyObject *b, int op);

static PyObject *vec4Copy(py_vec4 *self);
static PyObject *vec4Len2(py_vec4 *self);
static PyObject *vec4Len(py_vec4 *self);
static PyObject *vec4Dist2(py_vec4 *self, py_vec4 *o);
static PyObject *vec4Dist(py_vec4 *self, py_vec4 *o);
static PyObject *vec4ScaleToLen(py_vec4 *self, PyObject *o);
static PyObject *vec4ScaledToLen(py_vec4 *self, PyObject *o);
static PyObject *vec4Normalize(py_vec4 *self);
static PyObject *vec4Normalized(py_vec4 *self);

/** Number protocol functions */
static PyObject *vec4Add(py_vec4 *self, PyObject *o);
static PyObject *vec4Sub(py_vec4 *self, PyObject *o);
static PyObject *vec4Mul(py_vec4 *self, PyObject *o);
static PyObject *vec4Neg(py_vec4 *self);
static PyObject *vec4Div(py_vec4 *self, PyObject *o);
static PyObject *vec4AddIn(py_vec4 *self, PyObject *o);
static PyObject *vec4SubIn(py_vec4 *self, PyObject *o);
static PyObject *vec4MulIn(py_vec4 *self, PyObject *o);
static PyObject *vec4DivIn(py_vec4 *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t vec4SeqSize(py_vec4 *self);
static PyObject *vec4SeqGet(py_vec4 *self, Py_ssize_t i);
static int vec4SeqSet(py_vec4 *self, Py_ssize_t i, PyObject *val);

static PyGetSetDef py_vec4_getset[] = {
    {"x", (getter)vec4GetX, (setter)vec4SetX, "x coordinate", NULL},
    {"y", (getter)vec4GetY, (setter)vec4SetY, "y coordinate", NULL},
    {"z", (getter)vec4GetZ, (setter)vec4SetZ, "z coordinate", NULL},
    {"w", (getter)vec4GetW, (setter)vec4SetW, "w coordinate", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef py_vec4_methods[] = {
    { "copy", (PyCFunction)vec4Copy, METH_NOARGS,
      "Returns deep copy of Vec4." },
    { "len2", (PyCFunction)vec4Len2, METH_NOARGS,
      "len2() -> float\n"
      "Returns squared length of vector" },
    { "len", (PyCFunction)vec4Len, METH_NOARGS,
      "len() -> float\n"
      "Returns length of vector" },
    { "dist2", (PyCFunction)vec4Dist2, METH_O,
      "dist2(v : Vec4) -> float\n"
      "Squared distance of two vectors" },
    { "dist", (PyCFunction)vec4Dist, METH_O,
      "dist(v : Vec4) -> float\n"
      "Distance of two vectors" },
    { "scaleToLen", (PyCFunction)vec4ScaleToLen, METH_O,
      "scaleToLen(len : float) -> self\n"
      "Scale vector to given length" },
    { "scaledToLen", (PyCFunction)vec4ScaledToLen, METH_O,
      "scaledToLen(len : float) -> Vec4\n"
      "Returns vector scaled to given length" },
    { "normalize", (PyCFunction)vec4Normalize, METH_NOARGS,
      "normalize() -> self\n"
      "Normalize vector in-place" },
    { "normalized", (PyCFunction)vec4Normalized, METH_NOARGS,
      "normalized() -> Vec4\n"
      "Returns new normalized vector" },
    { NULL }
};

static PyNumberMethods py_vec4_num = {
    (binaryfunc)vec4Add, /* binaryfunc nb_add */
    (binaryfunc)vec4Sub, /* binaryfunc nb_subtract */
    (binaryfunc)vec4Mul, /* binaryfunc nb_multiply */
    0,                   /* binaryfunc nb_remainder */
    0,                   /* binaryfunc nb_divmod */
    0,                   /* ternaryfunc nb_power */
    (unaryfunc)vec4Neg,  /* unaryfunc nb_negative */
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

    (binaryfunc)vec4AddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)vec4SubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)vec4MulIn, /* binaryfunc nb_inplace_multiply */
    0,                     /* binaryfunc nb_inplace_remainder */
    0,                     /* ternaryfunc nb_inplace_power */
    0,                     /* binaryfunc nb_inplace_lshift */
    0,                     /* binaryfunc nb_inplace_rshift */
    0,                     /* binaryfunc nb_inplace_and */
    0,                     /* binaryfunc nb_inplace_xor */
    0,                     /* binaryfunc nb_inplace_or */

    (binaryfunc)vec4Div,   /* binaryfunc nb_floor_divide */
    (binaryfunc)vec4Div,   /* binaryfunc nb_true_divide */
    (binaryfunc)vec4DivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)vec4DivIn, /* binaryfunc nb_inplace_true_divide */

    0                      /* unaryfunc nb_index */
};

static PySequenceMethods py_vec4_seq = {
    (lenfunc)vec4SeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)vec4SeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)vec4SeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_vec4_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Vec4",             /* tp_name */
    sizeof(py_vec4),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)vec4AsStr,       /* tp_repr */
    &py_vec4_num,              /* tp_as_number */
    &py_vec4_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)vec4AsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "4D vector",               /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    vec4Cmp,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_vec4_methods,           /* tp_methods */
    0,                         /* tp_members */
    py_vec4_getset,            /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vec4ObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void vec4Init(PyObject *module)
{
    py_vec4_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_vec4_type) < 0)
        return;

    Py_INCREF(&py_vec4_type);
    PyModule_AddObject(module, "Vec4", (PyObject *)&py_vec4_type);
}


static int vec4ObjInit(py_vec4 *self, PyObject *_args, PyObject *kwds)
{
    fer_real_t v[4];
    Py_ssize_t i, len = 0;
    PyObject *args = NULL, *val;

    v[0] = FER_ZERO;
    v[1] = FER_ZERO;
    v[2] = FER_ZERO;
    v[3] = FER_ZERO;

    if (PySequence_Check(_args)){
        len = PySequence_Size(_args);
        args = PySequence_Fast(_args, "error");
    }

    for (i = 0; i < len && i < 4; i++){
        val = PySequence_Fast_GET_ITEM(args, i);
        if (PyFloat_Check(val)){
            v[i] = PyFloat_AsDouble(val);
        }else if (PyNumber_Check(val)){
            v[i] = numberAsReal(val);
        }
    }

    if (args){
        Py_DECREF(args);
    }

    ferVec4Set(&self->v, v[0], v[1], v[2], v[3]);
    return 0;
}

static PyObject *vec4AsStr(py_vec4 *self)
{
    char str[100];
    snprintf(str, 100, "<Vec4: %f %f %f %f>", 
             ferVec4X(&self->v), ferVec4Y(&self->v),
             ferVec4Z(&self->v), ferVec4W(&self->v));
    return PyUnicode_FromString(str);
}

static PyObject *vec4GetX(py_vec4 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec4X(&self->v));
}
static PyObject *vec4GetY(py_vec4 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec4Y(&self->v));
}
static PyObject *vec4GetZ(py_vec4 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec4Z(&self->v));
}
static PyObject *vec4GetW(py_vec4 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec4W(&self->v));
}

static int vec4SetX(py_vec4 *self, PyObject *val, void *coord)
{
    ferVec4SetX(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec4SetY(py_vec4 *self, PyObject *val, void *coord)
{
    ferVec4SetY(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec4SetZ(py_vec4 *self, PyObject *val, void *coord)
{
    ferVec4SetZ(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec4SetW(py_vec4 *self, PyObject *val, void *coord)
{
    ferVec4SetW(&self->v, PyFloat_AsDouble(val));
    return 0;
}


static PyObject *vec4Cmp(PyObject *a, PyObject *b, int op)
{
    py_vec4 *v1, *v2;

    if (!PyObject_TypeCheck(a, &py_vec4_type)
            || !PyObject_TypeCheck(b, &py_vec4_type)){
        return Py_NotImplemented;
    }

    v1 = (py_vec4 *)a;
    v2 = (py_vec4 *)b;
    if (op == Py_EQ){
        return ferVec4Eq(&v1->v, &v2->v) ? Py_True : Py_False;
    }else if (op == Py_NE){
        return ferVec4NEq(&v1->v, &v2->v) ? Py_True : Py_False;
    }
    return Py_NotImplemented;
}

static PyObject *vec4Copy(py_vec4 *self)
{
    py_vec4 *v = PyObject_New(py_vec4, &py_vec4_type);
    ferVec4Copy(&v->v, &self->v);
    return (PyObject *)v;
}

static PyObject *vec4Len2(py_vec4 *self)
{
    return PyFloat_FromDouble(ferVec4Len2(&self->v));
}
static PyObject *vec4Len(py_vec4 *self)
{
    return PyFloat_FromDouble(ferVec4Len(&self->v));
}

static PyObject *vec4Dist2(py_vec4 *self, py_vec4 *o)
{
    fer_real_t d;

    CHECK_VEC4(o)

    d = ferVec4Dist2(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}
static PyObject *vec4Dist(py_vec4 *self, py_vec4 *o)
{
    fer_real_t d;

    CHECK_VEC4(o)

    d = ferVec4Dist(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}

static PyObject *vec4ScaleToLen(py_vec4 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec4ScaleToLen(&self->v, num);
    Py_INCREF(self);
    return (PyObject *)self;
}
static PyObject *vec4ScaledToLen(py_vec4 *self, PyObject *o)
{
    fer_real_t num;
    py_vec4 *v;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec4, &py_vec4_type);
    ferVec4Copy(&v->v, &self->v);
    ferVec4ScaleToLen(&v->v, num);
    return (PyObject *)v;
}

static PyObject *vec4Normalize(py_vec4 *self)
{
    ferVec4Normalize(&self->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *vec4Normalized(py_vec4 *self)
{
    py_vec4 *v;
    v = PyObject_New(py_vec4, &py_vec4_type);
    ferVec4Copy(&v->v, &self->v);
    ferVec4Normalize(&v->v);
    return (PyObject *)v;
}




static PyObject *vec4Add(py_vec4 *self, PyObject *o)
{
    py_vec4 *v;
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec4_type)){
        v = PyObject_New(py_vec4, &py_vec4_type);
        ferVec4Add2(&v->v, &self->v, &((py_vec4 *)o)->v);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec4, &py_vec4_type);
        f = numberAsReal(o);

        ferVec4AddConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec4"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vec4Sub(py_vec4 *self, PyObject *o)
{
    py_vec4 *v;
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec4_type)){
        v = PyObject_New(py_vec4, &py_vec4_type);
        ferVec4Sub2(&v->v, &self->v, &((py_vec4 *)o)->v);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec4, &py_vec4_type);
        f = numberAsReal(o);

        ferVec4SubConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec4"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vec4Mul(py_vec4 *self, PyObject *o)
{
    py_vec4 *v;
    fer_real_t num;

    if (PyObject_TypeCheck(o, &py_vec4_type)){
        num = ferVec4Dot(&self->v, &((py_vec4 *)o)->v);
        return PyFloat_FromDouble(num);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec4, &py_vec4_type);
        num = numberAsReal(o);

        ferVec4Copy(&v->v, &self->v);
        ferVec4Scale(&v->v, num);
        return (PyObject *)v;
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected Vec4 or float number");
        return NULL;
    }
}

static PyObject *vec4Neg(py_vec4 *self)
{
    py_vec4 *v;

    v = PyObject_New(py_vec4, &py_vec4_type);
    ferVec4Copy(&v->v, &self->v);
    ferVec4Scale(&v->v, -FER_ONE);
    return (PyObject *)v;
}

static PyObject *vec4Div(py_vec4 *self, PyObject *o)
{
    py_vec4 *v;
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec4, &py_vec4_type);
    ferVec4Copy(&v->v, &self->v);
    ferVec4Scale(&v->v, FER_ONE / num);
    return (PyObject *)v;
}

static PyObject *vec4AddIn(py_vec4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec4_type)){
        ferVec4Add(&self->v, &((py_vec4 *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVec4AddConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec4SubIn(py_vec4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec4_type)){
        ferVec4Sub(&self->v, &((py_vec4 *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVec4SubConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec4MulIn(py_vec4 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec4Scale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec4DivIn(py_vec4 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec4Scale(&self->v, FER_ONE / num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


static Py_ssize_t vec4SeqSize(py_vec4 *self)
{
    return 4;
}

static PyObject *vec4SeqGet(py_vec4 *self, Py_ssize_t i)
{
    if (i >= 4){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferVec4Get(&self->v, i));
}

static int vec4SeqSet(py_vec4 *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 4){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferVec4SetCoord(&self->v, i, numberAsReal(val));
    return 0;
}
