// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTARRAYMETADATA_H
#define PY_AVTARRAYMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtArrayMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTARRAYMETADATA_NMETH (AVTVARMETADATA_NMETH+6)
void VISITPY_API           PyavtArrayMetaData_StartUp(avtArrayMetaData *subj, void *data);
void VISITPY_API           PyavtArrayMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtArrayMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtArrayMetaData_Check(PyObject *obj);
VISITPY_API avtArrayMetaData *  PyavtArrayMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtArrayMetaData_New();
VISITPY_API PyObject *     PyavtArrayMetaData_Wrap(const avtArrayMetaData *attr);
void VISITPY_API           PyavtArrayMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtArrayMetaData_SetDefaults(const avtArrayMetaData *atts);
std::string VISITPY_API    PyavtArrayMetaData_GetLogString();
std::string VISITPY_API    PyavtArrayMetaData_ToString(const avtArrayMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtArrayMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtArrayMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtArrayMetaData_methods[AVTARRAYMETADATA_NMETH];

#endif

