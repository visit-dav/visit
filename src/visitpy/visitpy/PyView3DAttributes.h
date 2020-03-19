// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEW3DATTRIBUTES_H
#define PY_VIEW3DATTRIBUTES_H
#include <Python.h>
#include <View3DAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEW3DATTRIBUTES_NMETH 39
void VISITPY_API           PyView3DAttributes_StartUp(View3DAttributes *subj, void *data);
void VISITPY_API           PyView3DAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyView3DAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyView3DAttributes_Check(PyObject *obj);
VISITPY_API View3DAttributes *  PyView3DAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyView3DAttributes_New();
VISITPY_API PyObject *     PyView3DAttributes_Wrap(const View3DAttributes *attr);
void VISITPY_API           PyView3DAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyView3DAttributes_SetDefaults(const View3DAttributes *atts);
std::string VISITPY_API    PyView3DAttributes_GetLogString();
std::string VISITPY_API    PyView3DAttributes_ToString(const View3DAttributes *, const char *);
VISITPY_API PyObject *     PyView3DAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyView3DAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyView3DAttributes_methods[VIEW3DATTRIBUTES_NMETH];

#endif

