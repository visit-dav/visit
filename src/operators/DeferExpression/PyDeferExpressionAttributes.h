// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DEFEREXPRESSIONATTRIBUTES_H
#define PY_DEFEREXPRESSIONATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DeferExpressionAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DEFEREXPRESSIONATTRIBUTES_NMETH 4
void           PyDeferExpressionAttributes_StartUp(DeferExpressionAttributes *subj, void *data);
void           PyDeferExpressionAttributes_CloseDown();
PyMethodDef *  PyDeferExpressionAttributes_GetMethodTable(int *nMethods);
bool           PyDeferExpressionAttributes_Check(PyObject *obj);
DeferExpressionAttributes *  PyDeferExpressionAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDeferExpressionAttributes_New();
PyObject *     PyDeferExpressionAttributes_Wrap(const DeferExpressionAttributes *attr);
void           PyDeferExpressionAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDeferExpressionAttributes_SetDefaults(const DeferExpressionAttributes *atts);
std::string    PyDeferExpressionAttributes_GetLogString();
std::string    PyDeferExpressionAttributes_ToString(const DeferExpressionAttributes *, const char *, const bool=false);
PyObject *     PyDeferExpressionAttributes_getattr(PyObject *self, char *name);
int            PyDeferExpressionAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDeferExpressionAttributes_methods[DEFEREXPRESSIONATTRIBUTES_NMETH];

#endif

