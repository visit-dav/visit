// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DBOPTIONSATTRIBUTES_H
#define PY_DBOPTIONSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DBOptionsAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define DBOPTIONSATTRIBUTES_NMETH 6
void VISITPY_API           PyDBOptionsAttributes_StartUp(DBOptionsAttributes *subj, void *data);
void VISITPY_API           PyDBOptionsAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyDBOptionsAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyDBOptionsAttributes_Check(PyObject *obj);
VISITPY_API DBOptionsAttributes *  PyDBOptionsAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyDBOptionsAttributes_New();
VISITPY_API PyObject *     PyDBOptionsAttributes_Wrap(const DBOptionsAttributes *attr);
void VISITPY_API           PyDBOptionsAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyDBOptionsAttributes_SetDefaults(const DBOptionsAttributes *atts);
std::string VISITPY_API    PyDBOptionsAttributes_GetLogString();
std::string VISITPY_API    PyDBOptionsAttributes_ToString(const DBOptionsAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyDBOptionsAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyDBOptionsAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyDBOptionsAttributes_methods[DBOPTIONSATTRIBUTES_NMETH];

#endif

