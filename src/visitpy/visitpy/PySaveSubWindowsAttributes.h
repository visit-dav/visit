// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SAVESUBWINDOWSATTRIBUTES_H
#define PY_SAVESUBWINDOWSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SaveSubWindowsAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SAVESUBWINDOWSATTRIBUTES_NMETH 34
void VISITPY_API           PySaveSubWindowsAttributes_StartUp(SaveSubWindowsAttributes *subj, void *data);
void VISITPY_API           PySaveSubWindowsAttributes_CloseDown();
VISITPY_API PyMethodDef *  PySaveSubWindowsAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PySaveSubWindowsAttributes_Check(PyObject *obj);
VISITPY_API SaveSubWindowsAttributes *  PySaveSubWindowsAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySaveSubWindowsAttributes_New();
VISITPY_API PyObject *     PySaveSubWindowsAttributes_Wrap(const SaveSubWindowsAttributes *attr);
void VISITPY_API           PySaveSubWindowsAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySaveSubWindowsAttributes_SetDefaults(const SaveSubWindowsAttributes *atts);
std::string VISITPY_API    PySaveSubWindowsAttributes_GetLogString();
std::string VISITPY_API    PySaveSubWindowsAttributes_ToString(const SaveSubWindowsAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PySaveSubWindowsAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PySaveSubWindowsAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySaveSubWindowsAttributes_methods[SAVESUBWINDOWSATTRIBUTES_NMETH];

#endif

