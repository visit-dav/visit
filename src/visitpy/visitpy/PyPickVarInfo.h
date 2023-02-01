// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PICKVARINFO_H
#define PY_PICKVARINFO_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PickVarInfo.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PICKVARINFO_NMETH 28
void VISITPY_API           PyPickVarInfo_StartUp(PickVarInfo *subj, void *data);
void VISITPY_API           PyPickVarInfo_CloseDown();
VISITPY_API PyMethodDef *  PyPickVarInfo_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPickVarInfo_Check(PyObject *obj);
VISITPY_API PickVarInfo *  PyPickVarInfo_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPickVarInfo_New();
VISITPY_API PyObject *     PyPickVarInfo_Wrap(const PickVarInfo *attr);
void VISITPY_API           PyPickVarInfo_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPickVarInfo_SetDefaults(const PickVarInfo *atts);
std::string VISITPY_API    PyPickVarInfo_GetLogString();
std::string VISITPY_API    PyPickVarInfo_ToString(const PickVarInfo *, const char *, const bool=false);
VISITPY_API PyObject *     PyPickVarInfo_getattr(PyObject *self, char *name);
int VISITPY_API            PyPickVarInfo_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPickVarInfo_methods[PICKVARINFO_NMETH];

#endif

