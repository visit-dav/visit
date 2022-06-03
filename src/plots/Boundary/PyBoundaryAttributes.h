// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_BOUNDARYATTRIBUTES_H
#define PY_BOUNDARYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <BoundaryAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define BOUNDARYATTRIBUTES_NMETH 24
void           PyBoundaryAttributes_StartUp(BoundaryAttributes *subj, void *data);
void           PyBoundaryAttributes_CloseDown();
PyMethodDef *  PyBoundaryAttributes_GetMethodTable(int *nMethods);
bool           PyBoundaryAttributes_Check(PyObject *obj);
BoundaryAttributes *  PyBoundaryAttributes_FromPyObject(PyObject *obj);
PyObject *     PyBoundaryAttributes_New();
PyObject *     PyBoundaryAttributes_Wrap(const BoundaryAttributes *attr);
void           PyBoundaryAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyBoundaryAttributes_SetDefaults(const BoundaryAttributes *atts);
std::string    PyBoundaryAttributes_GetLogString();
std::string    PyBoundaryAttributes_ToString(const BoundaryAttributes *, const char *, const bool=false);
PyObject *     PyBoundaryAttributes_getattr(PyObject *self, char *name);
int            PyBoundaryAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyBoundaryAttributes_methods[BOUNDARYATTRIBUTES_NMETH];

#endif

