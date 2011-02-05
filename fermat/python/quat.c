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
#include "vec3.h"
#include "quat.h"

static int quatObjInit(py_quat *self, PyObject *args, PyObject *kwds);
static PyObject *quatAsStr(py_quat *self);

/** Returns x, y, z coordinates */
static PyObject *quatGetX(py_quat *self, void *coord);
static PyObject *quatGetY(py_quat *self, void *coord);
static PyObject *quatGetZ(py_quat *self, void *coord);
static PyObject *quatGetW(py_quat *self, void *coord);
/** Sets x, y, z coordinates */
static int quatSetX(py_quat *self, PyObject *val, void *coord);
static int quatSetY(py_quat *self, PyObject *val, void *coord);
static int quatSetZ(py_quat *self, PyObject *val, void *coord);
static int quatSetW(py_quat *self, PyObject *val, void *coord);
/** Compare function */
static PyObject *quatCmp(PyObject *a, PyObject *b, int op);

static PyObject *quatCopy(py_quat *self);
static PyObject *quatLen2(py_quat *self);
static PyObject *quatLen(py_quat *self);
static PyObject *quatNormalize(py_quat *self);
static PyObject *quatNormalized(py_quat *self);
static PyObject *quatSetAngleAxis(py_quat *self, PyObject *o);
static PyObject *quatMul(py_quat *self, py_quat *o);
static PyObject *quatMuled(py_quat *self, py_quat *o);

/** Number protocol functions */
static PyObject *quatMulConst(py_quat *self, PyObject *o);
static PyObject *quatNeg(py_quat *self);
static PyObject *quatDiv(py_quat *self, PyObject *o);
static PyObject *quatMulIn(py_quat *self, PyObject *o);
static PyObject *quatDivIn(py_quat *self, PyObject *o);
static PyObject *quatInvert(py_quat *self);
static PyObject *quatInverted(py_quat *self);
static PyObject *quatRot(py_quat *self, py_vec3 *o);
static PyObject *quatRoted(py_quat *self, py_vec3 *o);

/** Sequence protocol functions */
static Py_ssize_t quatSeqSize(py_quat *self);
static PyObject *quatSeqGet(py_quat *self, Py_ssize_t i);
static int quatSeqSet(py_quat *self, Py_ssize_t i, PyObject *val);

static PyGetSetDef py_quat_getset[] = {
    {"x", (getter)quatGetX, (setter)quatSetX, "x coordinate", NULL},
    {"y", (getter)quatGetY, (setter)quatSetY, "y coordinate", NULL},
    {"z", (getter)quatGetZ, (setter)quatSetZ, "z coordinate", NULL},
    {"w", (getter)quatGetW, (setter)quatSetW, "w coordinate", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef py_quat_methods[] = {
    { "copy", (PyCFunction)quatCopy, METH_NOARGS,
      "Returns deep copy of Quat." },
    { "len2", (PyCFunction)quatLen2, METH_NOARGS,
      "len2() -> float\n"
      "Returns squared length of quaternion" },
    { "len", (PyCFunction)quatLen, METH_NOARGS,
      "len() -> float\n"
      "Returns length of quaternion" },
    { "normalize", (PyCFunction)quatNormalize, METH_NOARGS,
      "normalize() -> self\n"
      "Normalize quaternion in-place" },
    { "normalized", (PyCFunction)quatNormalized, METH_NOARGS,
      "normalized() -> Quat\n"
      "Returns new normalized quaternion" },
    { "setAngleAxis", (PyCFunction)quatSetAngleAxis, METH_VARARGS,
      "TODO" },
    { "mul", (PyCFunction)quatMul, METH_O,
      "Multiplies quternion with other one - this is composition of rotations." },
    { "muled", (PyCFunction)quatMuled, METH_O,
      "Returns multiplied quternions - this is composition of rotations." },
    { "invert", (PyCFunction)quatInvert, METH_NOARGS,
      "Inverts quaternion." },
    { "inverted", (PyCFunction)quatInverted, METH_NOARGS,
      "Returns inverted quaternion." },
    { "rot", (PyCFunction)quatRot, METH_O,
      "Rotates given Vec3." },
    { "roted", (PyCFunction)quatRoted, METH_O,
      "Returns rotated Vec3." },
    { NULL }
};

static PyNumberMethods py_quat_num = {
    0,                        /* binaryfunc nb_add */
    0,                        /* binaryfunc nb_subtract */
    (binaryfunc)quatMulConst, /* binaryfunc nb_multiply */
    0,                        /* binaryfunc nb_remainder */
    0,                        /* binaryfunc nb_divmod */
    0,                        /* ternaryfunc nb_power */
    (unaryfunc)quatNeg,       /* unaryfunc nb_negative */
    0,                        /* unaryfunc nb_positive */
    0,                        /* unaryfunc nb_absolute */
    0,                        /* inquiry nb_bool */
    0,                        /* unaryfunc nb_invert */
    0,                        /* binaryfunc nb_lshift */
    0,                        /* binaryfunc nb_rshift */
    0,                        /* binaryfunc nb_and */
    0,                        /* binaryfunc nb_xor */
    0,                        /* binaryfunc nb_or */
    0,                        /* unaryfunc nb_int */
    0,                        /* void *nb_reserved */
    0,                        /* unaryfunc nb_float */

    0,                        /* binaryfunc nb_inplace_add */
    0,                        /* binaryfunc nb_inplace_subtract */
    (binaryfunc)quatMulIn,    /* binaryfunc nb_inplace_multiply */
    0,                        /* binaryfunc nb_inplace_remainder */
    0,                        /* ternaryfunc nb_inplace_power */
    0,                        /* binaryfunc nb_inplace_lshift */
    0,                        /* binaryfunc nb_inplace_rshift */
    0,                        /* binaryfunc nb_inplace_and */
    0,                        /* binaryfunc nb_inplace_xor */
    0,                        /* binaryfunc nb_inplace_or */

    (binaryfunc)quatDiv,      /* binaryfunc nb_floor_divide */
    (binaryfunc)quatDiv,      /* binaryfunc nb_true_divide */
    (binaryfunc)quatDivIn,    /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)quatDivIn,    /* binaryfunc nb_inplace_true_divide */

    0                         /* unaryfunc nb_index */
};

static PySequenceMethods py_quat_seq = {
    (lenfunc)quatSeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)quatSeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)quatSeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_quat_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Quat",             /* tp_name */
    sizeof(py_quat),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    __ferDefaultDealloc,       /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)quatAsStr,       /* tp_repr */
    &py_quat_num,              /* tp_as_number */
    &py_quat_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)quatAsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Quaternion",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    quatCmp,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_quat_methods,           /* tp_methods */
    0,                         /* tp_members */
    py_quat_getset,            /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)quatObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void quatInit(PyObject *module)
{
    py_quat_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_quat_type) < 0)
        return;

    Py_INCREF(&py_quat_type);
    PyModule_AddObject(module, "Quat", (PyObject *)&py_quat_type);
}


static int quatObjInit(py_quat *self, PyObject *_args, PyObject *kwds)
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

    ferQuatSet(&self->v, v[0], v[1], v[2], v[3]);
    return 0;
}

static PyObject *quatAsStr(py_quat *self)
{
    char str[100];
    snprintf(str, 100, "<Quat: %f %f %f %f>", 
             ferQuatX(&self->v), ferQuatY(&self->v),
             ferQuatZ(&self->v), ferQuatW(&self->v));
    return PyUnicode_FromString(str);
}

static PyObject *quatGetX(py_quat *self, void *coord)
{
    return PyFloat_FromDouble(ferQuatX(&self->v));
}
static PyObject *quatGetY(py_quat *self, void *coord)
{
    return PyFloat_FromDouble(ferQuatY(&self->v));
}
static PyObject *quatGetZ(py_quat *self, void *coord)
{
    return PyFloat_FromDouble(ferQuatZ(&self->v));
}
static PyObject *quatGetW(py_quat *self, void *coord)
{
    return PyFloat_FromDouble(ferQuatW(&self->v));
}

static int quatSetX(py_quat *self, PyObject *val, void *coord)
{
    ferQuatSetX(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int quatSetY(py_quat *self, PyObject *val, void *coord)
{
    ferQuatSetY(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int quatSetZ(py_quat *self, PyObject *val, void *coord)
{
    ferQuatSetZ(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int quatSetW(py_quat *self, PyObject *val, void *coord)
{
    ferQuatSetW(&self->v, PyFloat_AsDouble(val));
    return 0;
}


static PyObject *quatCmp(PyObject *a, PyObject *b, int op)
{
    py_quat *v1, *v2;

    if (!PyObject_TypeCheck(a, &py_quat_type)
            || !PyObject_TypeCheck(b, &py_quat_type)){
        return Py_NotImplemented;
    }

    v1 = (py_quat *)a;
    v2 = (py_quat *)b;
    if (op == Py_EQ){
        return ferQuatEq(&v1->v, &v2->v) ? Py_True : Py_False;
    }else if (op == Py_NE){
        return ferQuatNEq(&v1->v, &v2->v) ? Py_True : Py_False;
    }
    return Py_NotImplemented;
}

static PyObject *quatCopy(py_quat *self)
{
    py_quat *v = PyObject_New(py_quat, &py_quat_type);
    ferQuatCopy(&v->v, &self->v);
    return (PyObject *)v;
}

static PyObject *quatLen2(py_quat *self)
{
    return PyFloat_FromDouble(ferQuatLen2(&self->v));
}
static PyObject *quatLen(py_quat *self)
{
    return PyFloat_FromDouble(ferQuatLen(&self->v));
}


static PyObject *quatNormalize(py_quat *self)
{
    ferQuatNormalize(&self->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *quatNormalized(py_quat *self)
{
    py_quat *v;
    v = PyObject_New(py_quat, &py_quat_type);
    ferQuatCopy(&v->v, &self->v);
    ferQuatNormalize(&v->v);
    return (PyObject *)v;
}

static PyObject *quatSetAngleAxis(py_quat *self, PyObject *o)
{
    py_vec3 *axis;
    double angle;

    if (!PyArg_ParseTuple(o, "dO", &angle, &axis)
            || !PyObject_TypeCheck(axis, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Method takes exactly two arguments (angle : float, axis : Vec3).");
        return NULL;
    }

    ferQuatSetAngleAxis(&self->v, angle, &axis->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *quatMul(py_quat *self, py_quat *o)
{
    CHECK_QUAT(o);

    ferQuatMul(&self->v, &o->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *quatMuled(py_quat *self, py_quat *o)
{
    py_quat *q;

    CHECK_QUAT(o);

    q = PyObject_New(py_quat, &py_quat_type);
    ferQuatMul2(&q->v, &self->v, &o->v);
    return (PyObject *)q;
}

static PyObject *quatInvert(py_quat *self)
{
    ferQuatInvert(&self->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *quatInverted(py_quat *self)
{
    py_quat *q;

    q = PyObject_New(py_quat, &py_quat_type);
    ferQuatInvert2(&q->v, &self->v);
    return (PyObject *)q;
}

static PyObject *quatRot(py_quat *self, py_vec3 *o)
{
    CHECK_VEC3(o);
    ferQuatRotVec(&o->v, &self->v);
    Py_INCREF(o);
    return (PyObject *)o;
}

static PyObject *quatRoted(py_quat *self, py_vec3 *o)
{
    py_vec3 *v;

    CHECK_VEC3(o);

    v = PyObject_New(py_vec3, &py_vec3_type);
    ferVec3Copy(&v->v, &o->v);
    ferQuatRotVec(&v->v, &self->v);
    return (PyObject *)v;
}


static PyObject *quatMulConst(py_quat *self, PyObject *o)
{
    py_quat *v;
    fer_real_t num;
    PyObject *tmp;

    if (PyNumber_Check((PyObject *)self)){
        tmp = (PyObject *)self;
        self = (py_quat *)o;
        o = tmp;
    }

    CHECK_FLOAT(o);

    v = PyObject_New(py_quat, &py_quat_type);
    num = numberAsReal(o);

    ferQuatCopy(&v->v, &self->v);
    ferQuatScale(&v->v, num);
    return (PyObject *)v;
}

static PyObject *quatNeg(py_quat *self)
{
    py_quat *v;

    v = PyObject_New(py_quat, &py_quat_type);
    ferQuatCopy(&v->v, &self->v);
    ferQuatScale(&v->v, -FER_ONE);
    return (PyObject *)v;
}

static PyObject *quatDiv(py_quat *self, PyObject *o)
{
    py_quat *v;
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_quat, &py_quat_type);
    ferQuatCopy(&v->v, &self->v);
    ferQuatScale(&v->v, FER_ONE / num);
    return (PyObject *)v;
}

static PyObject *quatMulIn(py_quat *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferQuatScale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *quatDivIn(py_quat *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferQuatScale(&self->v, FER_ONE / num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


static Py_ssize_t quatSeqSize(py_quat *self)
{
    return 4;
}

static PyObject *quatSeqGet(py_quat *self, Py_ssize_t i)
{
    if (i >= 4){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferQuatGet(&self->v, i));
}

static int quatSeqSet(py_quat *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 4){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferQuatSetCoord(&self->v, i, numberAsReal(val));
    return 0;
}
