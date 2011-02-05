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
#include "vec4.h"
#include "mat4.h"

static int mat4ObjInit(py_mat4 *self, PyObject *args, PyObject *kwds);
static PyObject *mat4AsStr(py_mat4 *self);

static PyObject *mat4Copy(py_mat4 *self);

static PyObject *mat4Get(py_mat4 *self, PyObject *args);

static PyObject *mat4Set(py_mat4 *self, PyObject *args);
static PyObject *mat4SetIdentity(py_mat4 *self);
static PyObject *mat4SetZero(py_mat4 *self);
static PyObject *mat4SetScale(py_mat4 *self, PyObject *s);
static PyObject *mat4SetTranslate(py_mat4 *self, py_vec3 *vec3);
static PyObject *mat4SetRot(py_mat4 *self, PyObject *args);

static PyObject *mat4TrScale(py_mat4 *self, PyObject *s);
static PyObject *mat4Translate(py_mat4 *self, py_vec3 *vec3);
static PyObject *mat4Rot(py_mat4 *self, PyObject *angle);

static PyObject *mat4Compose(py_mat4 *self, py_mat4 *mat);
static PyObject *mat4Composed(py_mat4 *self, py_mat4 *mat);
static PyObject *mat4MulComp(py_mat4 *self, py_mat4 *mat);
static PyObject *mat4MuledComp(py_mat4 *self, py_mat4 *mat);
static PyObject *mat4Trans(py_mat4 *self);
static PyObject *mat4Transed(py_mat4 *self);
static PyObject *mat4Regular(py_mat4 *self);
static PyObject *mat4Singular(py_mat4 *self);
static PyObject *mat4Det(py_mat4 *self);
static PyObject *mat4Inv(py_mat4 *self);
static PyObject *mat4Inved(py_mat4 *self);

/** Number protocol functions */
static PyObject *mat4Add(py_mat4 *self, PyObject *o);
static PyObject *mat4Sub(py_mat4 *self, PyObject *o);
static PyObject *mat4Mul(py_mat4 *self, PyObject *o);
static PyObject *mat4Neg(py_mat4 *self);
static PyObject *mat4Div(py_mat4 *self, PyObject *o);
static PyObject *mat4AddIn(py_mat4 *self, PyObject *o);
static PyObject *mat4SubIn(py_mat4 *self, PyObject *o);
static PyObject *mat4MulIn(py_mat4 *self, PyObject *o);
static PyObject *mat4DivIn(py_mat4 *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t mat4SeqSize(py_mat4 *self);
static PyObject *mat4SeqGet(py_mat4 *self, Py_ssize_t i);
static int mat4SeqSet(py_mat4 *self, Py_ssize_t i, PyObject *val);

static PyMethodDef py_mat4_methods[] = {
    { "copy", (PyCFunction)mat4Copy, METH_NOARGS,
      "Returns deep copy of Mat4." },
    { "set", (PyCFunction)mat4Set, METH_VARARGS,
      ".set(float),\n"
      ".set(row : int, col : int, val : float),\n"
      ".set(a, b, c, d, e, f, g, h, i)\n" },
    { "setIdentity", (PyCFunction)mat4SetIdentity, METH_NOARGS,
      "" },
    { "setZero", (PyCFunction)mat4SetZero, METH_NOARGS,
      "" },
    { "setScale", (PyCFunction)mat4SetScale, METH_O,
      "" },
    { "setTranslate", (PyCFunction)mat4SetTranslate, METH_O,
      "" },
    { "setRot", (PyCFunction)mat4SetRot, METH_VARARGS,
      "" },
    { "get", (PyCFunction)mat4Get, METH_VARARGS,
      "get(row : int, col : int)" },
    { "trScale", (PyCFunction)mat4TrScale, METH_O,
      "" },
    { "translate", (PyCFunction)mat4Translate, METH_O,
      "" },
    { "rot", (PyCFunction)mat4Rot, METH_VARARGS,
      "" },
    { "compose", (PyCFunction)mat4Compose, METH_O,
      "" },
    { "composed", (PyCFunction)mat4Composed, METH_O,
      "" },
    { "mulLeft", (PyCFunction)mat4Compose, METH_O,
      "" },
    { "muledLeft", (PyCFunction)mat4Composed, METH_O,
      "" },
    { "mulComp", (PyCFunction)mat4MulComp, METH_O,
      "" },
    { "muledComp", (PyCFunction)mat4MuledComp, METH_O,
      "" },
    { "transpose", (PyCFunction)mat4Trans, METH_NOARGS,
      "" },
    { "transposed", (PyCFunction)mat4Transed, METH_NOARGS,
      "" },
    { "regular", (PyCFunction)mat4Regular, METH_NOARGS,
      "" },
    { "singular", (PyCFunction)mat4Singular, METH_NOARGS,
      "" },
    { "det", (PyCFunction)mat4Det, METH_NOARGS,
      "" },
    { "determinant", (PyCFunction)mat4Det, METH_NOARGS,
      "" },
    { "inv", (PyCFunction)mat4Inv, METH_NOARGS,
      "" },
    { "invert", (PyCFunction)mat4Inv, METH_NOARGS,
      "" },
    { "inved", (PyCFunction)mat4Inved, METH_NOARGS,
      "" },
    { "inverted", (PyCFunction)mat4Inved, METH_NOARGS,
      "" },
    { NULL }
};

static PyNumberMethods py_mat4_num = {
    (binaryfunc)mat4Add, /* binaryfunc nb_add */
    (binaryfunc)mat4Sub, /* binaryfunc nb_subtract */
    (binaryfunc)mat4Mul, /* binaryfunc nb_multiply */
    0,                   /* binaryfunc nb_remainder */
    0,                   /* binaryfunc nb_divmod */
    0,                   /* ternaryfunc nb_power */
    (unaryfunc)mat4Neg,  /* unaryfunc nb_negative */
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

    (binaryfunc)mat4AddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)mat4SubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)mat4MulIn, /* binaryfunc nb_inplace_multiply */
    0,                     /* binaryfunc nb_inplace_remainder */
    0,                     /* ternaryfunc nb_inplace_power */
    0,                     /* binaryfunc nb_inplace_lshift */
    0,                     /* binaryfunc nb_inplace_rshift */
    0,                     /* binaryfunc nb_inplace_and */
    0,                     /* binaryfunc nb_inplace_xor */
    0,                     /* binaryfunc nb_inplace_or */

    (binaryfunc)mat4Div,   /* binaryfunc nb_floor_divide */
    (binaryfunc)mat4Div,   /* binaryfunc nb_true_divide */
    (binaryfunc)mat4DivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)mat4DivIn, /* binaryfunc nb_inplace_true_divide */

    0                      /* unaryfunc nb_index */
};

static PySequenceMethods py_mat4_seq = {
    (lenfunc)mat4SeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)mat4SeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)mat4SeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_mat4_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Mat4",             /* tp_name */
    sizeof(py_mat4),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    __ferDefaultDealloc,       /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)mat4AsStr,       /* tp_repr */
    &py_mat4_num,              /* tp_as_number */
    &py_mat4_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)mat4AsStr,       /* tp_str */
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
    py_mat4_methods,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)mat4ObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void mat4Init(PyObject *module)
{
    py_mat4_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_mat4_type) < 0)
        return;

    Py_INCREF(&py_mat4_type);
    PyModule_AddObject(module, "Mat4", (PyObject *)&py_mat4_type);
}


static int mat4ObjInit(py_mat4 *self, PyObject *_args, PyObject *kwds)
{
    PyObject *self2;

    self2 = mat4Set(self, _args);
    if (self2){
        Py_DECREF(self2);
        return 0;
    }

    return -1;
}

static PyObject *mat4AsStr(py_mat4 *self)
{
    char str[200];
    snprintf(str, 200, "<Mat4: %f %f %f %f | %f %f %f %f | %f %f %f %f | %f %f %f %f >", 
             ferMat4Get(&self->m, 0, 0), ferMat4Get(&self->m, 0, 1), ferMat4Get(&self->m, 0, 2), ferMat4Get(&self->m, 0, 3),
             ferMat4Get(&self->m, 1, 0), ferMat4Get(&self->m, 1, 1), ferMat4Get(&self->m, 1, 2), ferMat4Get(&self->m, 1, 3),
             ferMat4Get(&self->m, 2, 0), ferMat4Get(&self->m, 2, 1), ferMat4Get(&self->m, 2, 2), ferMat4Get(&self->m, 2, 3),
             ferMat4Get(&self->m, 3, 0), ferMat4Get(&self->m, 3, 1), ferMat4Get(&self->m, 3, 2), ferMat4Get(&self->m, 3, 3));
    return PyUnicode_FromString(str);
}


static PyObject *mat4Get(py_mat4 *self, PyObject *_args)
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
    if (r >= 4 || c >= 4){
        Py_DECREF(args);
        PyErr_SetString(PyExc_TypeError, "Expected int, int (row < 3, col < 3)");
        return NULL;
    }

    val = PyFloat_FromDouble(ferMat4Get(&self->m, r, c));

    if (args){
        Py_DECREF(args);
    }

    return val;
}

static PyObject *mat4Copy(py_mat4 *self)
{
    py_mat4 *m;

    m = PyObject_New(py_mat4, &py_mat4_type);
    ferMat4Copy(&m->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat4Set(py_mat4 *self, PyObject *_args)
{
    Py_ssize_t i, len = 0;
    PyObject *args = NULL, *val, *row, *col;
    fer_real_t f, fs[16];
    size_t r, c;

    len = PySequence_Size(_args);
    args = PySequence_Fast(_args, "error");

    if (len == 1){
        val = PySequence_Fast_GET_ITEM(args, 0);
        CHECK_FLOAT(val);
        f = numberAsReal(val);
        ferMat4SetAll(&self->m, f);
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
        ferMat4Set1(&self->m, r, c, f);
    }else if (len == 16){
        for (i = 0; i < 16; i++){
            val = PySequence_Fast_GET_ITEM(args, i);
            CHECK_FLOAT(val);

            fs[i] = numberAsReal(val);
        }
        ferMat4Set(&self->m, fs[0], fs[1], fs[2], fs[3],
                             fs[4], fs[5], fs[6], fs[7],
                             fs[8], fs[9], fs[10], fs[11],
                             fs[12], fs[13], fs[14], fs[15]);
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

static PyObject *mat4SetIdentity(py_mat4 *self)
{
    ferMat4SetIdentity(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4SetZero(py_mat4 *self)
{
    ferMat4SetZero(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4SetScale(py_mat4 *self, PyObject *s)
{
    CHECK_FLOAT(s);

    ferMat4SetScale(&self->m, numberAsReal(s));
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4SetTranslate(py_mat4 *self, py_vec3 *vec3)
{
    CHECK_VEC3(vec3);

    ferMat4SetTranslate(&self->m, &vec3->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4SetRot(py_mat4 *self, PyObject *args)
{
    fer_real_t angle;
    py_vec3 *axis;
    PyObject *o1, *o2;

    if (PyTuple_Size(args) == 2){
        o1 = PyTuple_GetItem(args, 0);
        o2 = PyTuple_GetItem(args, 1);
        if (PyNumber_Check(o1) && PyObject_TypeCheck(o2, &py_vec3_type)){
            angle = numberAsReal(o1);
            axis = (py_vec3 *)o2;
        }else if (PyNumber_Check(o2) && PyObject_TypeCheck(o1, &py_vec3_type)){
            angle = numberAsReal(o2);
            axis = (py_vec3 *)o1;
        }else{
            PyErr_SetString(PyExc_TypeError, "Expected float and Vec3 (angle, axis)");
            return NULL;
        }
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float and Vec3 (angle, axis)");
        return NULL;
    }

    ferMat4SetRot(&self->m, angle, &axis->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4TrScale(py_mat4 *self, PyObject *s)
{
    CHECK_FLOAT(s);

    ferMat4TrScale(&self->m, numberAsReal(s));
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Translate(py_mat4 *self, py_vec3 *vec3)
{
    CHECK_VEC3(vec3);

    ferMat4Translate(&self->m, &vec3->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Rot(py_mat4 *self, PyObject *args)
{
    fer_real_t angle;
    py_vec3 *axis;
    PyObject *o1, *o2;

    if (PyTuple_Size(args) == 2){
        o1 = PyTuple_GetItem(args, 0);
        o2 = PyTuple_GetItem(args, 1);
        if (PyNumber_Check(o1) && PyObject_TypeCheck(o2, &py_vec3_type)){
            angle = numberAsReal(o1);
            axis = (py_vec3 *)o2;
        }else if (PyNumber_Check(o2) && PyObject_TypeCheck(o1, &py_vec3_type)){
            angle = numberAsReal(o2);
            axis = (py_vec3 *)o1;
        }else{
            PyErr_SetString(PyExc_TypeError, "Expected float and Vec3 (angle, axis)");
            return NULL;
        }
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float and Vec3 (angle, axis)");
        return NULL;
    }

    ferMat4Rot(&self->m, angle, &axis->v);
    Py_INCREF(self);
    return (PyObject *)self;
}





static PyObject *mat4Compose(py_mat4 *self, py_mat4 *mat)
{
    CHECK_MAT4(mat);
    ferMat4Compose(&self->m, &mat->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Composed(py_mat4 *self, py_mat4 *mat)
{
    py_mat4 *m;

    CHECK_MAT4(mat);

    m = PyObject_New(py_mat4, &py_mat4_type);
    ferMat4Mul2(&m->m, &mat->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat4MulComp(py_mat4 *self, py_mat4 *mat)
{
    CHECK_MAT4(mat);
    ferMat4MulComp(&self->m, &mat->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4MuledComp(py_mat4 *self, py_mat4 *mat)
{
    py_mat4 *m;

    CHECK_MAT4(mat);

    m = PyObject_New(py_mat4, &py_mat4_type);
    ferMat4MulComp2(&m->m, &mat->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat4Trans(py_mat4 *self)
{
    ferMat4Trans(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Transed(py_mat4 *self)
{
    py_mat4 *m;

    m = PyObject_New(py_mat4, &py_mat4_type);
    ferMat4Trans2(&m->m, &self->m);

    return (PyObject *)m;
}

static PyObject *mat4Regular(py_mat4 *self)
{
    if (ferMat4Regular(&self->m)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *mat4Singular(py_mat4 *self)
{
    if (ferMat4Singular(&self->m)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *mat4Det(py_mat4 *self)
{
    fer_real_t det;

    det = ferMat4Det(&self->m);
    return PyFloat_FromDouble(det);
}

static PyObject *mat4Inv(py_mat4 *self)
{
    ferMat4Inv(&self->m);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Inved(py_mat4 *self)
{
    py_mat4 *m;

    m = PyObject_New(py_mat4, &py_mat4_type);
    ferMat4Inv2(&m->m, &self->m);

    return (PyObject *)m;
}





static PyObject *mat4Add(py_mat4 *self, PyObject *o)
{
    py_mat4 *m;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat4_type)){
        tmp = (PyObject *)self;
        self = (py_mat4 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        ferMat4Add2(&m->m, &self->m, &((py_mat4 *)o)->m);
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        f = numberAsReal(o);

        ferMat4AddConst2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat4Sub(py_mat4 *self, PyObject *o)
{
    py_mat4 *m;
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        ferMat4Sub2(&m->m, &self->m, &((py_mat4 *)o)->m);
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        f = numberAsReal(o);

        ferMat4SubConst2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat4Mul(py_mat4 *self, PyObject *o)
{
    py_mat4 *m;
    py_vec3 *v;
    py_vec4 *v4;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat4_type)){
        tmp = (PyObject *)self;
        self = (py_mat4 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        ferMat4Mul2(&m->m, &self->m, &((py_mat4 *)o)->m);
    }else if (PyObject_TypeCheck(o, &py_vec3_type)){
        v = PyObject_New(py_vec3, &py_vec3_type);
        ferMat4MulVec3(&v->v, &self->m, &((py_vec3 *)o)->v);
        return (PyObject *)v;
    }else if (PyObject_TypeCheck(o, &py_vec4_type)){
        v4 = PyObject_New(py_vec4, &py_vec4_type);
        ferMat4MulVec(&v4->v, &self->m, &((py_vec4 *)o)->v);
        return (PyObject *)v4;
    }else if (PyNumber_Check(o)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        f = numberAsReal(o);

        ferMat4Scale2(&m->m, &self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float, Vec2, Vec3 or Mat4"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat4Neg(py_mat4 *self)
{
    ferMat4Scale(&self->m, -FER_ONE);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *mat4Div(py_mat4 *self, PyObject *o)
{
    py_mat4 *m;
    fer_real_t f;
    PyObject *tmp;

    if (!PyObject_TypeCheck(self, &py_mat4_type)){
        tmp = (PyObject *)self;
        self = (py_mat4 *)o;
        o = tmp;
    }

    if (PyNumber_Check(o)){
        m = PyObject_New(py_mat4, &py_mat4_type);
        f = numberAsReal(o);

        ferMat4Scale2(&m->m, &self->m, ferRecp(f));
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float"); \
        return NULL;
    }

    return (PyObject *)m;
}

static PyObject *mat4AddIn(py_mat4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        ferMat4Add(&self->m, &((py_mat4 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat4AddConst(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat4SubIn(py_mat4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        ferMat4Sub(&self->m, &((py_mat4 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat4SubConst(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat4MulIn(py_mat4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_mat4_type)){
        ferMat4Mul(&self->m, &((py_mat4 *)o)->m);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat4Scale(&self->m, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *mat4DivIn(py_mat4 *self, PyObject *o)
{
    fer_real_t f;

    if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferMat4Scale(&self->m, ferRecp(f));
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Mat4"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


/** Sequence protocol functions */
static Py_ssize_t mat4SeqSize(py_mat4 *self)
{
    return 16;
}

static PyObject *mat4SeqGet(py_mat4 *self, Py_ssize_t i)
{
    if (i >= 16){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferMat4Get(&self->m, i / 3, i % 3));
}

static int mat4SeqSet(py_mat4 *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 16){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferMat4Set1(&self->m, i / 4, i % 4, numberAsReal(val));
    return 0;
}

