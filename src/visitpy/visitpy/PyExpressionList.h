// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXPRESSIONLIST_H
#define PY_EXPRESSIONLIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExpressionList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define EXPRESSIONLIST_NMETH 7
void VISITPY_API           PyExpressionList_StartUp(ExpressionList *subj, void *data);
void VISITPY_API           PyExpressionList_CloseDown();
VISITPY_API PyMethodDef *  PyExpressionList_GetMethodTable(int *nMethods);
bool VISITPY_API           PyExpressionList_Check(PyObject *obj);
VISITPY_API ExpressionList *  PyExpressionList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyExpressionList_New();
VISITPY_API PyObject *     PyExpressionList_Wrap(const ExpressionList *attr);
void VISITPY_API           PyExpressionList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyExpressionList_SetDefaults(const ExpressionList *atts);
std::string VISITPY_API    PyExpressionList_GetLogString();
std::string VISITPY_API    PyExpressionList_ToString(const ExpressionList *, const char *, const bool=false);
VISITPY_API PyObject *     PyExpressionList_getattr(PyObject *self, char *name);
int VISITPY_API            PyExpressionList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyExpressionList_methods[EXPRESSIONLIST_NMETH];

#endif

