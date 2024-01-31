// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_GLOBALATTRIBUTES_H
#define PY_GLOBALATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <GlobalAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define GLOBALATTRIBUTES_NMETH 58
void VISITPY_API           PyGlobalAttributes_StartUp(GlobalAttributes *subj, void *data);
void VISITPY_API           PyGlobalAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyGlobalAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyGlobalAttributes_Check(PyObject *obj);
VISITPY_API GlobalAttributes *  PyGlobalAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyGlobalAttributes_New();
VISITPY_API PyObject *     PyGlobalAttributes_Wrap(const GlobalAttributes *attr);
void VISITPY_API           PyGlobalAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyGlobalAttributes_SetDefaults(const GlobalAttributes *atts);
std::string VISITPY_API    PyGlobalAttributes_GetLogString();
std::string VISITPY_API    PyGlobalAttributes_ToString(const GlobalAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyGlobalAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyGlobalAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyGlobalAttributes_methods[GLOBALATTRIBUTES_NMETH];

#endif

