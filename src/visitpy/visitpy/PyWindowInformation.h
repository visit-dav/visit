// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_WINDOWINFORMATION_H
#define PY_WINDOWINFORMATION_H
#include <Python.h>
#include <Py2and3Support.h>
#include <WindowInformation.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define WINDOWINFORMATION_NMETH 60
void VISITPY_API           PyWindowInformation_StartUp(WindowInformation *subj, void *data);
void VISITPY_API           PyWindowInformation_CloseDown();
VISITPY_API PyMethodDef *  PyWindowInformation_GetMethodTable(int *nMethods);
bool VISITPY_API           PyWindowInformation_Check(PyObject *obj);
VISITPY_API WindowInformation *  PyWindowInformation_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyWindowInformation_New();
VISITPY_API PyObject *     PyWindowInformation_Wrap(const WindowInformation *attr);
void VISITPY_API           PyWindowInformation_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyWindowInformation_SetDefaults(const WindowInformation *atts);
std::string VISITPY_API    PyWindowInformation_GetLogString();
std::string VISITPY_API    PyWindowInformation_ToString(const WindowInformation *, const char *, const bool=false);
VISITPY_API PyObject *     PyWindowInformation_getattr(PyObject *self, char *name);
int VISITPY_API            PyWindowInformation_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyWindowInformation_methods[WINDOWINFORMATION_NMETH];

#endif

