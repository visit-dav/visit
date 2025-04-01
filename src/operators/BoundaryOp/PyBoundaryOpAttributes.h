// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_BOUNDARYOPATTRIBUTES_H
#define PY_BOUNDARYOPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <BoundaryOpAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define BOUNDARYOPATTRIBUTES_NMETH 5
void           PyBoundaryOpAttributes_StartUp(BoundaryOpAttributes *subj, void *data);
void           PyBoundaryOpAttributes_CloseDown();
PyMethodDef *  PyBoundaryOpAttributes_GetMethodTable(int *nMethods);
bool           PyBoundaryOpAttributes_Check(PyObject *obj);
BoundaryOpAttributes *  PyBoundaryOpAttributes_FromPyObject(PyObject *obj);
PyObject *     PyBoundaryOpAttributes_New();
PyObject *     PyBoundaryOpAttributes_Wrap(const BoundaryOpAttributes *attr);
void           PyBoundaryOpAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyBoundaryOpAttributes_SetDefaults(const BoundaryOpAttributes *atts);
std::string    PyBoundaryOpAttributes_GetLogString();
std::string    PyBoundaryOpAttributes_ToString(const BoundaryOpAttributes *, const char *, const bool=false);
PyObject *     PyBoundaryOpAttributes_getattro(PyObject *self, PyObject *attr_name);
int            PyBoundaryOpAttributes_setattro(PyObject *self, PyObject *attr_name, PyObject *args);
extern PyMethodDef PyBoundaryOpAttributes_methods[BOUNDARYOPATTRIBUTES_NMETH];

#endif

