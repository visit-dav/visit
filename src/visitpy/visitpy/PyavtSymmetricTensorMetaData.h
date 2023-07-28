// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSYMMETRICTENSORMETADATA_H
#define PY_AVTSYMMETRICTENSORMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtSymmetricTensorMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSYMMETRICTENSORMETADATA_NMETH (AVTVARMETADATA_NMETH+4)
void VISITPY_API           PyavtSymmetricTensorMetaData_StartUp(avtSymmetricTensorMetaData *subj, void *data);
void VISITPY_API           PyavtSymmetricTensorMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtSymmetricTensorMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtSymmetricTensorMetaData_Check(PyObject *obj);
VISITPY_API avtSymmetricTensorMetaData *  PyavtSymmetricTensorMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtSymmetricTensorMetaData_New();
VISITPY_API PyObject *     PyavtSymmetricTensorMetaData_Wrap(const avtSymmetricTensorMetaData *attr);
void VISITPY_API           PyavtSymmetricTensorMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtSymmetricTensorMetaData_SetDefaults(const avtSymmetricTensorMetaData *atts);
std::string VISITPY_API    PyavtSymmetricTensorMetaData_GetLogString();
std::string VISITPY_API    PyavtSymmetricTensorMetaData_ToString(const avtSymmetricTensorMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtSymmetricTensorMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtSymmetricTensorMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtSymmetricTensorMetaData_methods[AVTSYMMETRICTENSORMETADATA_NMETH];

#endif

