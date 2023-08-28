// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTVECTORMETADATA_H
#define PY_AVTVECTORMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtVectorMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTVECTORMETADATA_NMETH (AVTVARMETADATA_NMETH+4)
void VISITPY_API           PyavtVectorMetaData_StartUp(avtVectorMetaData *subj, void *data);
void VISITPY_API           PyavtVectorMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtVectorMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtVectorMetaData_Check(PyObject *obj);
VISITPY_API avtVectorMetaData *  PyavtVectorMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtVectorMetaData_New();
VISITPY_API PyObject *     PyavtVectorMetaData_Wrap(const avtVectorMetaData *attr);
void VISITPY_API           PyavtVectorMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtVectorMetaData_SetDefaults(const avtVectorMetaData *atts);
std::string VISITPY_API    PyavtVectorMetaData_GetLogString();
std::string VISITPY_API    PyavtVectorMetaData_ToString(const avtVectorMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtVectorMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtVectorMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtVectorMetaData_methods[AVTVECTORMETADATA_NMETH];

#endif

