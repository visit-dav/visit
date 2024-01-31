// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTVARMETADATA_H
#define PY_AVTVARMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtVarMetaData.h>
#include <PyavtBaseVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTVARMETADATA_NMETH (AVTBASEVARMETADATA_NMETH+16)
void VISITPY_API           PyavtVarMetaData_StartUp(avtVarMetaData *subj, void *data);
void VISITPY_API           PyavtVarMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtVarMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtVarMetaData_Check(PyObject *obj);
VISITPY_API avtVarMetaData *  PyavtVarMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtVarMetaData_New();
VISITPY_API PyObject *     PyavtVarMetaData_Wrap(const avtVarMetaData *attr);
void VISITPY_API           PyavtVarMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtVarMetaData_SetDefaults(const avtVarMetaData *atts);
std::string VISITPY_API    PyavtVarMetaData_GetLogString();
std::string VISITPY_API    PyavtVarMetaData_ToString(const avtVarMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtVarMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtVarMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtVarMetaData_methods[AVTVARMETADATA_NMETH];

#endif

