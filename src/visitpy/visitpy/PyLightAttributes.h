// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LIGHTATTRIBUTES_H
#define PY_LIGHTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LightAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define LIGHTATTRIBUTES_NMETH 12
void VISITPY_API           PyLightAttributes_StartUp(LightAttributes *subj, void *data);
void VISITPY_API           PyLightAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyLightAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyLightAttributes_Check(PyObject *obj);
VISITPY_API LightAttributes *  PyLightAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyLightAttributes_New();
VISITPY_API PyObject *     PyLightAttributes_Wrap(const LightAttributes *attr);
void VISITPY_API           PyLightAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyLightAttributes_SetDefaults(const LightAttributes *atts);
std::string VISITPY_API    PyLightAttributes_GetLogString();
std::string VISITPY_API    PyLightAttributes_ToString(const LightAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyLightAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyLightAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyLightAttributes_methods[LIGHTATTRIBUTES_NMETH];

#endif

