// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_QUERYOVERTIMEATTRIBUTES_H
#define PY_QUERYOVERTIMEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <QueryOverTimeAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define QUERYOVERTIMEATTRIBUTES_NMETH 24
void VISITPY_API           PyQueryOverTimeAttributes_StartUp(QueryOverTimeAttributes *subj, void *data);
void VISITPY_API           PyQueryOverTimeAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyQueryOverTimeAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyQueryOverTimeAttributes_Check(PyObject *obj);
VISITPY_API QueryOverTimeAttributes *  PyQueryOverTimeAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyQueryOverTimeAttributes_New();
VISITPY_API PyObject *     PyQueryOverTimeAttributes_Wrap(const QueryOverTimeAttributes *attr);
void VISITPY_API           PyQueryOverTimeAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyQueryOverTimeAttributes_SetDefaults(const QueryOverTimeAttributes *atts);
std::string VISITPY_API    PyQueryOverTimeAttributes_GetLogString();
std::string VISITPY_API    PyQueryOverTimeAttributes_ToString(const QueryOverTimeAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyQueryOverTimeAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyQueryOverTimeAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyQueryOverTimeAttributes_methods[QUERYOVERTIMEATTRIBUTES_NMETH];

#endif

