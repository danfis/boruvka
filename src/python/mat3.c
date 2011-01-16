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
#include "vec2.h"
#include "vec3.h"
#include "mat3.h"

static int mat3ObjInit(py_mat3 *self, PyObject *args, PyObject *kwds);
static PyObject *mat3AsStr(py_mat3 *self);

static PyObject *mat3Copy(py_mat3 *self);

static PyObject *mat3Get(py_mat3 *self, PyObject *args);

static PyObject *mat3Set(py_mat3 *self, PyObject *args);
static PyObject *mat3SetIdentity(py_mat3 *self);
static PyObject *mat3SetZero(py_mat3 *self);
static PyObject *mat3SetScale(py_mat3 *self, PyObject *s);
static PyObject *mat3SetTranslate(py_mat3 *self, py_vec2 *vec2);
static PyObject *mat3SetRot(py_mat3 *self, PyObject *angle);

static PyObject *mat3TrScale(py_mat3 *self, PyObject *s);
static PyObject *mat3Translate(py_mat3 *self, py_vec2 *vec2);
static PyObject *mat3Rot(py_mat3 *self, PyObject *angle);

static PyObject *mat3Compose(py_mat3 *self, py_mat3 *mat);
static PyObject *mat3Composed(py_mat3 *self, py_mat3 *mat);
static PyObject *mat3MulComp(py_mat3 *self, py_mat3 *mat);
static PyObject *mat3MuledComp(py_mat3 *self, py_mat3 *mat);
static PyObject *mat3Trans(py_mat3 *self);
static PyObject *mat3Transed(py_mat3 *self);
static PyObject *mat3Regular(py_mat3 *self);
static PyObject *mat3Singular(py_mat3 *self);
static PyObject *mat3Det(py_mat3 *self);
static PyObject *mat3Inv(py_mat3 *self);
static PyObject *mat3Inved(py_mat3 *self);

/** Number protocol functions */
static PyObject *mat3Add(py_mat3 *self, PyObject *o);
static PyObject *mat3Sub(py_mat3 *self, PyObject *o);
static PyObject *mat3Mul(py_mat3 *self, PyObject *o);
static PyObject *mat3Neg(py_mat3 *self);
static PyObject *mat3Div(py_mat3 *self, PyObject *o);
static PyObject *mat3AddIn(py_mat3 *self, PyObject *o);
static PyObject *mat3SubIn(py_mat3 *self, PyObject *o);
static PyObject *mat3MulIn(py_mat3 *self, PyObject *o);
static PyObject *mat3DivIn(py_mat3 *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t mat3SeqSize(py_mat3 *self);
static PyObject *mat3SeqGet(py_mat3 *self, Py_ssize_t i);
static int mat3SeqSet(py_mat3 *self, Py_ssize_t i, PyObject *val);

static PyMethodDef py_mat3_methods[] = {
    { "copy", (PyCFunction)mat3Copy, METH_NOARGS,
      "Returns deep copy of Mat3." },
    { "set", (PyCFunction)mat3Set, METH_VARARGS,
      ".set(float),\n"
      ".set(row : int, col : int, val : float),\n"
      ".set(a, b, c, d, e, f, g, h, i)\n" },
    { "setIdentity", (PyCFunction)mat3SetIdentity, METH_NOARGS,
      "" },
    { "setZero", (PyCFunction)mat3SetZero, METH_NOARGS,
      "" },
    { "setScale", (PyCFunction)mat3SetScale, METH_O,
      "" },
    { "setTranslate", (PyCFunction)mat3SetTranslate, METH_O,
      "" },
    { "setRot", (PyCFunction)mat3SetRot, METH_O,
      "" },
    { "get", (PyCFunction)mat3Get, METH_VARARGS,
      "get(row : int, col : int)" },
    { "trScale", (PyCFunction)mat3TrScale, METH_O,
      "" },
    { "translate", (PyCFunction)mat3Translate, METH_O,
      "" },
    { "rot", (PyCFunction)mat3Rot, METH_O,
      "" },
    { "compose", (PyCFunction)mat3Compose, METH_O,
      "" },
    { "composed", (PyCFunction)mat3Composed, METH_O,
      "" },
    { "mulLeft", (PyCFunction)mat3Compose, METH_O,
      "" },
    { "muledLeft", (PyCFunction)mat3Composed, METH_O,
      "" },
    { "mulComp", (PyCFunction)mat3MulComp, METH_O,
      "" },
    { "muledComp", (PyCFunction)mat3MuledComp, METH_O,
      "" },
    { "transpose", (PyCFunction)mat3Trans, METH_NOARGS,
      "" },
    { "transposed", (PyCFunction)mat3Transed, METH_NOARGS,
      "" },
    { "regular", (PyCFunction)mat3Regular, METH_NOARGS,
      "" },
    { "singular", (PyCFunction)mat3Singular, METH_NOARGS,
      "" },
    { "det", (PyCFunction)mat3Det, METH_NOARGS,
      "" },
    { "determinant", (PyCFunction)mat3Det, METH_NOARGS,
      "" },
    { "inv", (PyCFunction)mat3Inv, METH_NOARGS,
      "" },
    { "invert", (PyCFunction)mat3Inv, METH_NOARGS,
      "" },
    { "inved", (PyCFunction)mat3Inved, METH_NOARGS,
      "" },
    { "inverted", (PyCFunction)mat3Inved, METH_NOARGS,
      "" },
    { NULL }
};

static PyNumberMethods py_mat3_num = {
    (binaryfunc)mat3Add, /* binaryfunc nb_add */
    (binaryfunc)mat3Sub, /* binaryfunc nb_subtract */
    (binaryfunc)mat3Mul, /* binaryfunc nb_multiply */
    0,                   /* binaryfunc nb_remainder */
    0,                   /* binaryfunc nb_divmod */
    0,                   /* ternaryfunc nb_power */
    (unaryfunc)mat3Neg,  /* unaryfunc nb_negative */
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

    (binaryfunc)mat3AddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)mat3SubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)mat3MulIn, /* binaryfunc nb_inplace_multiply */
    0,                     /* binaryfunc nb_inplace_remainder */
    0,                     /* ternaryfunc nb_inplace_power */
    0,                     /* binaryfunc nb_inplace_lshift */
    0,                     /* binaryfunc nb_inplace_rshift */
    0,                     /* binaryfunc nb_inplace_and */
    0,                     /* binaryfunc nb_inplace_xor */
    0,                     /* binaryfunc nb_inplace_or */

    (binaryfunc)mat3Div,   /* binaryfunc nb_floor_divide */
    (binaryfunc)mat3Div,   /* binaryfunc nb_true_divide */
    (binaryfunc)mat3DivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)mat3DivIn, /* binaryfunc nb_inplace_true_divide */

    0                      /* unaryfunc nb_index */
};

static PySequenceMethods py_mat3_seq = {
    (lenfunc)mat3SeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)mat3SeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)mat3SeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_mat3_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Mat3",             /* tp_name */
    sizeof(py_mat3),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)mat3AsStr,       /* tp_repr */
    &py_mat3_num,              /* tp_as_number */
    &py_mat3_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)mat3AsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "3x3 matrix",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_mat3_methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)mat3ObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void mat3Init(PyObject *module)
{
    py_mat3_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_mat3_type) < 0)
        return;

    Py_INCREF(&py_mat3_type);
    PyModule_AddObject(module, "Mat3", (PyObject *)&py_mat3_type);
}


static int mat3ObjInit(py_mat3 *self, PyObject *_args, PyObject *kwds)
{
    PyObject *self2;

    self2 = mat3Set(self, _args);
    if (self2){
        Py_DECREF(self2);
        return 0;
    }

    return -1;
}

static PyObject *mat3AsStr(py_mat3 *self)
{
    char str[200];
    snprintf(str, 200, "<Mat3: %f %f %f | %f %f %f | %f %f %f>", 
             ferMat3Get(&self->m, 0, 0), ferMat3Get(&self->m, 0, 1), ferMat3Get(&self->m, 0, 2),
             ferMat3Get(&self->m, 1, 0), ferMat3Get(&self->m, 1, 1), ferMat3Get(&self->m, 1, 2),
             ferMat3Get(&self->m, 2, 0), ferMat3Get(&self->m, 2, 1), ferMat3Get(&self->m, 2, 2));
    return PyUnicode_FromString(str);
}


static PyObject *mat3Get(py_mat3 *self, PyObject *_args)
{
    Py_ssize_t len = 0;
    PyObject *args = NULL, *val, *row, *col;
    size_t r, c;

    if (PySequence_Check(_args)){
        len = PySequence_Size(_args);
        args = PySequence_Fast(_args, "error");
    }

    if (len != 2){
        Py_DECREF(args);
        PyErr_SetString(PyExc_TypeError, "Expected int, int (row, col)");
        return NULL;
    }

    row = PySequence_Fast_GET_ITEM(args, 0);
    col = PySequence_Fast_GET_ITEM(args, 1);
    r = numberAsLong(row);
    c = numberAsLong(col);
    if (r >= 3 || c >= 3){
        Py_DECREF(args);
        PyErr_SetString(PyExc_TypeError, "Expected int, int (row < 3, col < 3)");
        return NULL;
    }

    val = PyFloat_FromDouble(ferMat3Get(&self->m, r, c));

    if (args){
        Py_DECREF(args);
    }

    return val;
}

static PyObject *mat3Copy(py_mat3 *self)
{
    py_mat3 *m;

    m = PyObject_New(py_mat3, &py_mat3_type);
    ferMat3Copy(&m->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat3Set(py_mat3 *self, PyObject *_args)
{
    Py_ssize_t i, len = 0;
    PyObject *args = NULL, *val, *row, *col;
    fer_real_t f, fs[9];
    size_t r, c;

    len = PySequence_Size(_args);
    args = PySequence_Fast(_args, "error");

    if (len == 1){
        val = PySequence_Fast_GET_ITEM(args, 0);
        CHECK_FLOAT(val);
        f = numberAsReal(val);
        ferMat3SetAll(&self->m, f);
    }else if (len == 3){
        row = PySequence_Fast_GET_ITEM(args, 0);
        col = PySequence_Fast_GET_ITEM(args, 1);
        val = PySequence_Fast_GET_ITEM(args, 2);
        if (!PyNumber_Check(row) || !PyNumber_Check(col) || !PyNumber_Check(val)){
            Py_DECREF(args);
            PyErr_SetString(PyExc_TypeError, "Expected int, int, float (row, col, val)");
            return NULL;
        }

        r = numberAsLong(row);
        c = numberAsLong(col);
        f = numberAsReal(val);
        ferMat3Set1(&self->m, r, c, f);
    }else if (len == 9){
        for (i = 0; i < 9; i++){
            val = PySequence_Fast_GET_ITEM(args, i);
            CHECK_FLOAT(val);

            fs[i] = numberAsReal(val);
        }
        ferMat3Set(&self->m, fs[0], fs[1], fs[2],
                             fs[3], fs[4], fs[5],
                             fs[6], fs[7], fs[8]);
    }else if (len != 0){
        Py_DECREF(args);
        PyErr_SetString(PyExc_TypeError, "Invalid arguments.");
        return NULL;
    }

    if (args){
        Py_DECREF(args);
    }

    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3SetIdentity(py_mat3 *self)
{
    ferMat3SetIdentity(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3SetZero(py_mat3 *self)
{
    ferMat3SetZero(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3SetScale(py_mat3 *self, PyObject *s)
{
    CHECK_FLOAT(s);

    ferMat3SetScale(&self->m, numberAsReal(s));
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3SetTranslate(py_mat3 *self, py_vec2 *vec2)
{
    CHECK_VEC2(vec2);

    ferMat3SetTranslate(&self->m, &vec2->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3SetRot(py_mat3 *self, PyObject *angle)
{
    CHECK_FLOAT(angle);

    ferMat3SetRot(&self->m, numberAsReal(angle));
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3TrScale(py_mat3 *self, PyObject *s)
{
    CHECK_FLOAT(s);

    ferMat3TrScale(&self->m, numberAsReal(s));
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Translate(py_mat3 *self, py_vec2 *vec2)
{
    CHECK_VEC2(vec2);

    ferMat3Translate(&self->m, &vec2->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Rot(py_mat3 *self, PyObject *angle)
{
    CHECK_FLOAT(angle);

    ferMat3Rot(&self->m, numberAsReal(angle));
    Py_INCREF(self);
    return (PyObject *)self;
}





static PyObject *mat3Compose(py_mat3 *self, py_mat3 *mat)
{
    CHECK_MAT3(mat);
    ferMat3Compose(&self->m, &mat->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Composed(py_mat3 *self, py_mat3 *mat)
{
    py_mat3 *m;

    CHECK_MAT3(mat);

    m = PyObject_New(py_mat3, &py_mat3_type);
    ferMat3Mul2(&m->m, &mat->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat3MulComp(py_mat3 *self, py_mat3 *mat)
{
    CHECK_MAT3(mat);
    ferMat3MulComp(&self->m, &mat->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3MuledComp(py_mat3 *self, py_mat3 *mat)
{
    py_mat3 *m;

    CHECK_MAT3(mat);

    m = PyObject_New(py_mat3, &py_mat3_type);
    ferMat3MulComp2(&m->m, &mat->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat3Trans(py_mat3 *self)
{
    ferMat3Trans(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Transed(py_mat3 *self)
{
    py_mat3 *m;

    m = PyObject_New(py_mat3, &py_mat3_type);
    ferMat3Trans2(&m->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat3Regular(py_mat3 *self)
{
    if (ferMat3Regular(&self->m)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *mat3Singular(py_mat3 *self)
{
    if (ferMat3Singular(&self->m)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *mat3Det(py_mat3 *self)
{
    fer_real_t det;

    det = ferMat3Det(&self->m);
    return PyFloat_FromDouble(det);
}

static PyObject *mat3Inv(py_mat3 *self)
{
    ferMat3Inv(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Inved(py_mat3 *self)
{
    py_mat3 *m;

    m = PyObject_New(py_mat3, &py_mat3_type);
    ferMat3Inv2(&m->m, &self->m);

    return (PyObject *)m;
}





static PyObject *mat3Add(py_mat3 *self, PyObject *o)
{
    py_mat3 *m;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat3_type)){
        tmp = (PyObject *)self;
        self = (py_mat3 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        ferMat3Add2(&m->m, &self->m, &((py_mat3 *)o)->m);
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        f = numberAsReal(o);

        ferMat3AddConst2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat3Sub(py_mat3 *self, PyObject *o)
{
    py_mat3 *m;
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        ferMat3Sub2(&m->m, &self->m, &((py_mat3 *)o)->m);
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        f = numberAsReal(o);

        ferMat3SubConst2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat3Mul(py_mat3 *self, PyObject *o)
{
    py_mat3 *m;
    py_vec2 *v;
    py_vec3 *v3;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat3_type)){
        tmp = (PyObject *)self;
        self = (py_mat3 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        ferMat3Mul2(&m->m, &self->m, &((py_mat3 *)o)->m);
    }else if (PyObject_TypeCheck(o, &py_vec2_type)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        ferMat3MulVec2(&v->v, &self->m, &((py_vec2 *)o)->v);
        return (PyObject *)v;
    }else if (PyObject_TypeCheck(o, &py_vec3_type)){
        v3 = PyObject_New(py_vec3, &py_vec3_type);
        ferMat3MulVec(&v3->v, &self->m, &((py_vec3 *)o)->v);
        return (PyObject *)v3;
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        f = numberAsReal(o);

        ferMat3Scale2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float, Vec2, Vec3 or Mat3"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat3Neg(py_mat3 *self)
{
    ferMat3Scale(&self->m, -FER_ONE);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat3Div(py_mat3 *self, PyObject *o)
{
    py_mat3 *m;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat3_type)){
        tmp = (PyObject *)self;
        self = (py_mat3 *)o;
        o = tmp;
    }

    if (PyNumber_Check(o)){
        m = PyObject_New(py_mat3, &py_mat3_type);
        f = numberAsReal(o);

        ferMat3Scale2(&m->m, &self->m, ferRecp(f));
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat3AddIn(py_mat3 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        ferMat3Add(&self->m, &((py_mat3 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat3AddConst(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat3SubIn(py_mat3 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        ferMat3Sub(&self->m, &((py_mat3 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat3SubConst(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat3MulIn(py_mat3 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat3_type)){
        ferMat3Mul(&self->m, &((py_mat3 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat3Scale(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat3DivIn(py_mat3 *self, PyObject *o)
{
    fer_real_t f;

    if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat3Scale(&self->m, ferRecp(f));
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat3"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


/** Sequence protocol functions */
static Py_ssize_t mat3SeqSize(py_mat3 *self)
{
    return 9;
}

static PyObject *mat3SeqGet(py_mat3 *self, Py_ssize_t i)
{
    if (i >= 9){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferMat3Get(&self->m, i / 3, i % 3));
}

static int mat3SeqSet(py_mat3 *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 9){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferMat3Set1(&self->m, i / 3, i % 3, numberAsReal(val));
    return 0;
}

