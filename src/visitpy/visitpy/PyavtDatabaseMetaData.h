// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTDATABASEMETADATA_H
#define PY_AVTDATABASEMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtDatabaseMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTDATABASEMETADATA_NMETH 110
void VISITPY_API           PyavtDatabaseMetaData_StartUp(avtDatabaseMetaData *subj, void *data);
void VISITPY_API           PyavtDatabaseMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtDatabaseMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtDatabaseMetaData_Check(PyObject *obj);
VISITPY_API avtDatabaseMetaData *  PyavtDatabaseMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtDatabaseMetaData_New();
VISITPY_API PyObject *     PyavtDatabaseMetaData_Wrap(const avtDatabaseMetaData *attr);
void VISITPY_API           PyavtDatabaseMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtDatabaseMetaData_SetDefaults(const avtDatabaseMetaData *atts);
std::string VISITPY_API    PyavtDatabaseMetaData_GetLogString();
std::string VISITPY_API    PyavtDatabaseMetaData_ToString(const avtDatabaseMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtDatabaseMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtDatabaseMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtDatabaseMetaData_methods[AVTDATABASEMETADATA_NMETH];

#endif

