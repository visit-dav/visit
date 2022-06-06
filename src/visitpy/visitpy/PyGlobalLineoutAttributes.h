// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_GLOBALLINEOUTATTRIBUTES_H
#define PY_GLOBALLINEOUTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <GlobalLineoutAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define GLOBALLINEOUTATTRIBUTES_NMETH 20
void VISITPY_API           PyGlobalLineoutAttributes_StartUp(GlobalLineoutAttributes *subj, void *data);
void VISITPY_API           PyGlobalLineoutAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyGlobalLineoutAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyGlobalLineoutAttributes_Check(PyObject *obj);
VISITPY_API GlobalLineoutAttributes *  PyGlobalLineoutAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyGlobalLineoutAttributes_New();
VISITPY_API PyObject *     PyGlobalLineoutAttributes_Wrap(const GlobalLineoutAttributes *attr);
void VISITPY_API           PyGlobalLineoutAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyGlobalLineoutAttributes_SetDefaults(const GlobalLineoutAttributes *atts);
std::string VISITPY_API    PyGlobalLineoutAttributes_GetLogString();
std::string VISITPY_API    PyGlobalLineoutAttributes_ToString(const GlobalLineoutAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyGlobalLineoutAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyGlobalLineoutAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyGlobalLineoutAttributes_methods[GLOBALLINEOUTATTRIBUTES_NMETH];

#endif

