// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTMESHMETADATA_H
#define PY_AVTMESHMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtMeshMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTMESHMETADATA_NMETH 106
void VISITPY_API           PyavtMeshMetaData_StartUp(avtMeshMetaData *subj, void *data);
void VISITPY_API           PyavtMeshMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtMeshMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtMeshMetaData_Check(PyObject *obj);
VISITPY_API avtMeshMetaData *  PyavtMeshMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtMeshMetaData_New();
VISITPY_API PyObject *     PyavtMeshMetaData_Wrap(const avtMeshMetaData *attr);
void VISITPY_API           PyavtMeshMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtMeshMetaData_SetDefaults(const avtMeshMetaData *atts);
std::string VISITPY_API    PyavtMeshMetaData_GetLogString();
std::string VISITPY_API    PyavtMeshMetaData_ToString(const avtMeshMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtMeshMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtMeshMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtMeshMetaData_methods[AVTMESHMETADATA_NMETH];

#endif

