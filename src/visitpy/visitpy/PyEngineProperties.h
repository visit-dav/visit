// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ENGINEPROPERTIES_H
#define PY_ENGINEPROPERTIES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <EngineProperties.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define ENGINEPROPERTIES_NMETH 12
void VISITPY_API           PyEngineProperties_StartUp(EngineProperties *subj, void *data);
void VISITPY_API           PyEngineProperties_CloseDown();
VISITPY_API PyMethodDef *  PyEngineProperties_GetMethodTable(int *nMethods);
bool VISITPY_API           PyEngineProperties_Check(PyObject *obj);
VISITPY_API EngineProperties *  PyEngineProperties_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyEngineProperties_New();
VISITPY_API PyObject *     PyEngineProperties_Wrap(const EngineProperties *attr);
void VISITPY_API           PyEngineProperties_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyEngineProperties_SetDefaults(const EngineProperties *atts);
std::string VISITPY_API    PyEngineProperties_GetLogString();
std::string VISITPY_API    PyEngineProperties_ToString(const EngineProperties *, const char *, const bool=false);
VISITPY_API PyObject *     PyEngineProperties_getattr(PyObject *self, char *name);
int VISITPY_API            PyEngineProperties_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyEngineProperties_methods[ENGINEPROPERTIES_NMETH];

#endif

