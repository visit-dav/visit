// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SAVEWINDOWATTRIBUTES_H
#define PY_SAVEWINDOWATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SaveWindowAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SAVEWINDOWATTRIBUTES_NMETH 42
void VISITPY_API           PySaveWindowAttributes_StartUp(SaveWindowAttributes *subj, void *data);
void VISITPY_API           PySaveWindowAttributes_CloseDown();
VISITPY_API PyMethodDef *  PySaveWindowAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PySaveWindowAttributes_Check(PyObject *obj);
VISITPY_API SaveWindowAttributes *  PySaveWindowAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySaveWindowAttributes_New();
VISITPY_API PyObject *     PySaveWindowAttributes_Wrap(const SaveWindowAttributes *attr);
void VISITPY_API           PySaveWindowAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySaveWindowAttributes_SetDefaults(const SaveWindowAttributes *atts);
std::string VISITPY_API    PySaveWindowAttributes_GetLogString();
std::string VISITPY_API    PySaveWindowAttributes_ToString(const SaveWindowAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PySaveWindowAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PySaveWindowAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySaveWindowAttributes_methods[SAVEWINDOWATTRIBUTES_NMETH];

#endif

