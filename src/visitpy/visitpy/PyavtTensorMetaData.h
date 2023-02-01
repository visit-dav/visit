// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTTENSORMETADATA_H
#define PY_AVTTENSORMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtTensorMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTTENSORMETADATA_NMETH (AVTVARMETADATA_NMETH+4)
void VISITPY_API           PyavtTensorMetaData_StartUp(avtTensorMetaData *subj, void *data);
void VISITPY_API           PyavtTensorMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtTensorMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtTensorMetaData_Check(PyObject *obj);
VISITPY_API avtTensorMetaData *  PyavtTensorMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtTensorMetaData_New();
VISITPY_API PyObject *     PyavtTensorMetaData_Wrap(const avtTensorMetaData *attr);
void VISITPY_API           PyavtTensorMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtTensorMetaData_SetDefaults(const avtTensorMetaData *atts);
std::string VISITPY_API    PyavtTensorMetaData_GetLogString();
std::string VISITPY_API    PyavtTensorMetaData_ToString(const avtTensorMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtTensorMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtTensorMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtTensorMetaData_methods[AVTTENSORMETADATA_NMETH];

#endif

