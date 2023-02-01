// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTMATERIALMETADATA_H
#define PY_AVTMATERIALMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtMaterialMetaData.h>
#include <PyavtBaseVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTMATERIALMETADATA_NMETH (AVTBASEVARMETADATA_NMETH+8)
void VISITPY_API           PyavtMaterialMetaData_StartUp(avtMaterialMetaData *subj, void *data);
void VISITPY_API           PyavtMaterialMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtMaterialMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtMaterialMetaData_Check(PyObject *obj);
VISITPY_API avtMaterialMetaData *  PyavtMaterialMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtMaterialMetaData_New();
VISITPY_API PyObject *     PyavtMaterialMetaData_Wrap(const avtMaterialMetaData *attr);
void VISITPY_API           PyavtMaterialMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtMaterialMetaData_SetDefaults(const avtMaterialMetaData *atts);
std::string VISITPY_API    PyavtMaterialMetaData_GetLogString();
std::string VISITPY_API    PyavtMaterialMetaData_ToString(const avtMaterialMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtMaterialMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtMaterialMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtMaterialMetaData_methods[AVTMATERIALMETADATA_NMETH];

#endif

