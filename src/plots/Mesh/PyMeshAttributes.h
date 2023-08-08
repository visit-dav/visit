// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MESHATTRIBUTES_H
#define PY_MESHATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MeshAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define MESHATTRIBUTES_NMETH 24
void           PyMeshAttributes_StartUp(MeshAttributes *subj, void *data);
void           PyMeshAttributes_CloseDown();
PyMethodDef *  PyMeshAttributes_GetMethodTable(int *nMethods);
bool           PyMeshAttributes_Check(PyObject *obj);
MeshAttributes *  PyMeshAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMeshAttributes_New();
PyObject *     PyMeshAttributes_Wrap(const MeshAttributes *attr);
void           PyMeshAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMeshAttributes_SetDefaults(const MeshAttributes *atts);
std::string    PyMeshAttributes_GetLogString();
std::string    PyMeshAttributes_ToString(const MeshAttributes *, const char *, const bool=false);
PyObject *     PyMeshAttributes_getattr(PyObject *self, char *name);
int            PyMeshAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMeshAttributes_methods[MESHATTRIBUTES_NMETH];

#endif

