// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SELECTIONSUMMARY_H
#define PY_SELECTIONSUMMARY_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SelectionSummary.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SELECTIONSUMMARY_NMETH 19
void VISITPY_API           PySelectionSummary_StartUp(SelectionSummary *subj, void *data);
void VISITPY_API           PySelectionSummary_CloseDown();
VISITPY_API PyMethodDef *  PySelectionSummary_GetMethodTable(int *nMethods);
bool VISITPY_API           PySelectionSummary_Check(PyObject *obj);
VISITPY_API SelectionSummary *  PySelectionSummary_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySelectionSummary_New();
VISITPY_API PyObject *     PySelectionSummary_Wrap(const SelectionSummary *attr);
void VISITPY_API           PySelectionSummary_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySelectionSummary_SetDefaults(const SelectionSummary *atts);
std::string VISITPY_API    PySelectionSummary_GetLogString();
std::string VISITPY_API    PySelectionSummary_ToString(const SelectionSummary *, const char *, const bool=false);
VISITPY_API PyObject *     PySelectionSummary_getattr(PyObject *self, char *name);
int VISITPY_API            PySelectionSummary_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySelectionSummary_methods[SELECTIONSUMMARY_NMETH];

#endif

