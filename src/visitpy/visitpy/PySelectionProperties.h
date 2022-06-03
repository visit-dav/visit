// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SELECTIONPROPERTIES_H
#define PY_SELECTIONPROPERTIES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SelectionProperties.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SELECTIONPROPERTIES_NMETH 40
void VISITPY_API           PySelectionProperties_StartUp(SelectionProperties *subj, void *data);
void VISITPY_API           PySelectionProperties_CloseDown();
VISITPY_API PyMethodDef *  PySelectionProperties_GetMethodTable(int *nMethods);
bool VISITPY_API           PySelectionProperties_Check(PyObject *obj);
VISITPY_API SelectionProperties *  PySelectionProperties_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySelectionProperties_New();
VISITPY_API PyObject *     PySelectionProperties_Wrap(const SelectionProperties *attr);
void VISITPY_API           PySelectionProperties_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySelectionProperties_SetDefaults(const SelectionProperties *atts);
std::string VISITPY_API    PySelectionProperties_GetLogString();
std::string VISITPY_API    PySelectionProperties_ToString(const SelectionProperties *, const char *, const bool=false);
VISITPY_API PyObject *     PySelectionProperties_getattr(PyObject *self, char *name);
int VISITPY_API            PySelectionProperties_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySelectionProperties_methods[SELECTIONPROPERTIES_NMETH];

#endif

