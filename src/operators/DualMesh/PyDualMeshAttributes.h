// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DUALMESHATTRIBUTES_H
#define PY_DUALMESHATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DualMeshAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DUALMESHATTRIBUTES_NMETH 4
void           PyDualMeshAttributes_StartUp(DualMeshAttributes *subj, void *data);
void           PyDualMeshAttributes_CloseDown();
PyMethodDef *  PyDualMeshAttributes_GetMethodTable(int *nMethods);
bool           PyDualMeshAttributes_Check(PyObject *obj);
DualMeshAttributes *  PyDualMeshAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDualMeshAttributes_New();
PyObject *     PyDualMeshAttributes_Wrap(const DualMeshAttributes *attr);
void           PyDualMeshAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDualMeshAttributes_SetDefaults(const DualMeshAttributes *atts);
std::string    PyDualMeshAttributes_GetLogString();
std::string    PyDualMeshAttributes_ToString(const DualMeshAttributes *, const char *, const bool=false);
PyObject *     PyDualMeshAttributes_getattr(PyObject *self, char *name);
int            PyDualMeshAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDualMeshAttributes_methods[DUALMESHATTRIBUTES_NMETH];

#endif

