// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FILLEDBOUNDARYATTRIBUTES_H
#define PY_FILLEDBOUNDARYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FilledBoundaryAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define FILLEDBOUNDARYATTRIBUTES_NMETH 40
void           PyFilledBoundaryAttributes_StartUp(FilledBoundaryAttributes *subj, void *data);
void           PyFilledBoundaryAttributes_CloseDown();
PyMethodDef *  PyFilledBoundaryAttributes_GetMethodTable(int *nMethods);
bool           PyFilledBoundaryAttributes_Check(PyObject *obj);
FilledBoundaryAttributes *  PyFilledBoundaryAttributes_FromPyObject(PyObject *obj);
PyObject *     PyFilledBoundaryAttributes_New();
PyObject *     PyFilledBoundaryAttributes_Wrap(const FilledBoundaryAttributes *attr);
void           PyFilledBoundaryAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyFilledBoundaryAttributes_SetDefaults(const FilledBoundaryAttributes *atts);
std::string    PyFilledBoundaryAttributes_GetLogString();
std::string    PyFilledBoundaryAttributes_ToString(const FilledBoundaryAttributes *, const char *, const bool=false);
PyObject *     PyFilledBoundaryAttributes_getattr(PyObject *self, char *name);
int            PyFilledBoundaryAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyFilledBoundaryAttributes_methods[FILLEDBOUNDARYATTRIBUTES_NMETH];

#endif

