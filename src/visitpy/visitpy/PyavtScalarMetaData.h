// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSCALARMETADATA_H
#define PY_AVTSCALARMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtScalarMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSCALARMETADATA_NMETH (AVTVARMETADATA_NMETH+30)
void VISITPY_API           PyavtScalarMetaData_StartUp(avtScalarMetaData *subj, void *data);
void VISITPY_API           PyavtScalarMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtScalarMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtScalarMetaData_Check(PyObject *obj);
VISITPY_API avtScalarMetaData *  PyavtScalarMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtScalarMetaData_New();
VISITPY_API PyObject *     PyavtScalarMetaData_Wrap(const avtScalarMetaData *attr);
void VISITPY_API           PyavtScalarMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtScalarMetaData_SetDefaults(const avtScalarMetaData *atts);
std::string VISITPY_API    PyavtScalarMetaData_GetLogString();
std::string VISITPY_API    PyavtScalarMetaData_ToString(const avtScalarMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtScalarMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtScalarMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtScalarMetaData_methods[AVTSCALARMETADATA_NMETH];

#endif

