// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PICKATTRIBUTES_H
#define PY_PICKATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PickAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PICKATTRIBUTES_NMETH 68
void VISITPY_API           PyPickAttributes_StartUp(PickAttributes *subj, void *data);
void VISITPY_API           PyPickAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyPickAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPickAttributes_Check(PyObject *obj);
VISITPY_API PickAttributes *  PyPickAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPickAttributes_New();
VISITPY_API PyObject *     PyPickAttributes_Wrap(const PickAttributes *attr);
void VISITPY_API           PyPickAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPickAttributes_SetDefaults(const PickAttributes *atts);
std::string VISITPY_API    PyPickAttributes_GetLogString();
std::string VISITPY_API    PyPickAttributes_ToString(const PickAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyPickAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyPickAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPickAttributes_methods[PICKATTRIBUTES_NMETH];

#endif

