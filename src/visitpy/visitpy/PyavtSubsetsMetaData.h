// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSUBSETSMETADATA_H
#define PY_AVTSUBSETSMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtSubsetsMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSUBSETSMETADATA_NMETH (AVTVARMETADATA_NMETH+26)
void VISITPY_API           PyavtSubsetsMetaData_StartUp(avtSubsetsMetaData *subj, void *data);
void VISITPY_API           PyavtSubsetsMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtSubsetsMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtSubsetsMetaData_Check(PyObject *obj);
VISITPY_API avtSubsetsMetaData *  PyavtSubsetsMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtSubsetsMetaData_New();
VISITPY_API PyObject *     PyavtSubsetsMetaData_Wrap(const avtSubsetsMetaData *attr);
void VISITPY_API           PyavtSubsetsMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtSubsetsMetaData_SetDefaults(const avtSubsetsMetaData *atts);
std::string VISITPY_API    PyavtSubsetsMetaData_GetLogString();
std::string VISITPY_API    PyavtSubsetsMetaData_ToString(const avtSubsetsMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtSubsetsMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtSubsetsMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtSubsetsMetaData_methods[AVTSUBSETSMETADATA_NMETH];

#endif

