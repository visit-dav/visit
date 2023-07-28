// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXPRESSION_H
#define PY_EXPRESSION_H
#include <Python.h>
#include <Py2and3Support.h>
#include <Expression.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define EXPRESSION_NMETH 22
void VISITPY_API           PyExpression_StartUp(Expression *subj, void *data);
void VISITPY_API           PyExpression_CloseDown();
VISITPY_API PyMethodDef *  PyExpression_GetMethodTable(int *nMethods);
bool VISITPY_API           PyExpression_Check(PyObject *obj);
VISITPY_API Expression *  PyExpression_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyExpression_New();
VISITPY_API PyObject *     PyExpression_Wrap(const Expression *attr);
void VISITPY_API           PyExpression_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyExpression_SetDefaults(const Expression *atts);
std::string VISITPY_API    PyExpression_GetLogString();
std::string VISITPY_API    PyExpression_ToString(const Expression *, const char *, const bool=false);
VISITPY_API PyObject *     PyExpression_getattr(PyObject *self, char *name);
int VISITPY_API            PyExpression_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyExpression_methods[EXPRESSION_NMETH];

#endif

