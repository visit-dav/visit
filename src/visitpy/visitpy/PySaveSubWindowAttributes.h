// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SAVESUBWINDOWATTRIBUTES_H
#define PY_SAVESUBWINDOWATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SaveSubWindowAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SAVESUBWINDOWATTRIBUTES_NMETH 12
void VISITPY_API           PySaveSubWindowAttributes_StartUp(SaveSubWindowAttributes *subj, void *data);
void VISITPY_API           PySaveSubWindowAttributes_CloseDown();
VISITPY_API PyMethodDef *  PySaveSubWindowAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PySaveSubWindowAttributes_Check(PyObject *obj);
VISITPY_API SaveSubWindowAttributes *  PySaveSubWindowAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySaveSubWindowAttributes_New();
VISITPY_API PyObject *     PySaveSubWindowAttributes_Wrap(const SaveSubWindowAttributes *attr);
void VISITPY_API           PySaveSubWindowAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySaveSubWindowAttributes_SetDefaults(const SaveSubWindowAttributes *atts);
std::string VISITPY_API    PySaveSubWindowAttributes_GetLogString();
std::string VISITPY_API    PySaveSubWindowAttributes_ToString(const SaveSubWindowAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PySaveSubWindowAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PySaveSubWindowAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySaveSubWindowAttributes_methods[SAVESUBWINDOWATTRIBUTES_NMETH];

#endif

