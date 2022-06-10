// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWATTRIBUTES_H
#define PY_VIEWATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWATTRIBUTES_NMETH 30
void VISITPY_API           PyViewAttributes_StartUp(ViewAttributes *subj, void *data);
void VISITPY_API           PyViewAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyViewAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewAttributes_Check(PyObject *obj);
VISITPY_API ViewAttributes *  PyViewAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewAttributes_New();
VISITPY_API PyObject *     PyViewAttributes_Wrap(const ViewAttributes *attr);
void VISITPY_API           PyViewAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewAttributes_SetDefaults(const ViewAttributes *atts);
std::string VISITPY_API    PyViewAttributes_GetLogString();
std::string VISITPY_API    PyViewAttributes_ToString(const ViewAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewAttributes_methods[VIEWATTRIBUTES_NMETH];

#endif

