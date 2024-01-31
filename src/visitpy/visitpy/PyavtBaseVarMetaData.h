// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTBASEVARMETADATA_H
#define PY_AVTBASEVARMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtBaseVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTBASEVARMETADATA_NMETH 12
void VISITPY_API           PyavtBaseVarMetaData_StartUp(avtBaseVarMetaData *subj, void *data);
void VISITPY_API           PyavtBaseVarMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtBaseVarMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtBaseVarMetaData_Check(PyObject *obj);
VISITPY_API avtBaseVarMetaData *  PyavtBaseVarMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtBaseVarMetaData_New();
VISITPY_API PyObject *     PyavtBaseVarMetaData_Wrap(const avtBaseVarMetaData *attr);
void VISITPY_API           PyavtBaseVarMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtBaseVarMetaData_SetDefaults(const avtBaseVarMetaData *atts);
std::string VISITPY_API    PyavtBaseVarMetaData_GetLogString();
std::string VISITPY_API    PyavtBaseVarMetaData_ToString(const avtBaseVarMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtBaseVarMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtBaseVarMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtBaseVarMetaData_methods[AVTBASEVARMETADATA_NMETH];

#endif

