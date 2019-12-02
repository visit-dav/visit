// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEW2DATTRIBUTES_H
#define PY_VIEW2DATTRIBUTES_H
#include <Python.h>
#include <View2DAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEW2DATTRIBUTES_NMETH 18
void VISITPY_API           PyView2DAttributes_StartUp(View2DAttributes *subj, void *data);
void VISITPY_API           PyView2DAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyView2DAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyView2DAttributes_Check(PyObject *obj);
VISITPY_API View2DAttributes *  PyView2DAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyView2DAttributes_New();
VISITPY_API PyObject *     PyView2DAttributes_Wrap(const View2DAttributes *attr);
void VISITPY_API           PyView2DAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyView2DAttributes_SetDefaults(const View2DAttributes *atts);
std::string VISITPY_API    PyView2DAttributes_GetLogString();
std::string VISITPY_API    PyView2DAttributes_ToString(const View2DAttributes *, const char *);
VISITPY_API PyObject *     PyView2DAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyView2DAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyView2DAttributes_methods[VIEW2DATTRIBUTES_NMETH];

#endif

