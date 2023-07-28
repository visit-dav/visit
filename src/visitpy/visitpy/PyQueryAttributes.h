// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_QUERYATTRIBUTES_H
#define PY_QUERYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <QueryAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define QUERYATTRIBUTES_NMETH 20
void VISITPY_API           PyQueryAttributes_StartUp(QueryAttributes *subj, void *data);
void VISITPY_API           PyQueryAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyQueryAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyQueryAttributes_Check(PyObject *obj);
VISITPY_API QueryAttributes *  PyQueryAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyQueryAttributes_New();
VISITPY_API PyObject *     PyQueryAttributes_Wrap(const QueryAttributes *attr);
void VISITPY_API           PyQueryAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyQueryAttributes_SetDefaults(const QueryAttributes *atts);
std::string VISITPY_API    PyQueryAttributes_GetLogString();
std::string VISITPY_API    PyQueryAttributes_ToString(const QueryAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyQueryAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyQueryAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyQueryAttributes_methods[QUERYATTRIBUTES_NMETH];

#endif

