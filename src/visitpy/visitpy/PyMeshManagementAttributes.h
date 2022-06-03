// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MESHMANAGEMENTATTRIBUTES_H
#define PY_MESHMANAGEMENTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MeshManagementAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define MESHMANAGEMENTATTRIBUTES_NMETH 16
void VISITPY_API           PyMeshManagementAttributes_StartUp(MeshManagementAttributes *subj, void *data);
void VISITPY_API           PyMeshManagementAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyMeshManagementAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyMeshManagementAttributes_Check(PyObject *obj);
VISITPY_API MeshManagementAttributes *  PyMeshManagementAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyMeshManagementAttributes_New();
VISITPY_API PyObject *     PyMeshManagementAttributes_Wrap(const MeshManagementAttributes *attr);
void VISITPY_API           PyMeshManagementAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyMeshManagementAttributes_SetDefaults(const MeshManagementAttributes *atts);
std::string VISITPY_API    PyMeshManagementAttributes_GetLogString();
std::string VISITPY_API    PyMeshManagementAttributes_ToString(const MeshManagementAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyMeshManagementAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyMeshManagementAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyMeshManagementAttributes_methods[MESHMANAGEMENTATTRIBUTES_NMETH];

#endif

