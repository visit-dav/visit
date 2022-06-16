// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SELECTIONVARIABLESUMMARY_H
#define PY_SELECTIONVARIABLESUMMARY_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SelectionVariableSummary.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SELECTIONVARIABLESUMMARY_NMETH 10
void VISITPY_API           PySelectionVariableSummary_StartUp(SelectionVariableSummary *subj, void *data);
void VISITPY_API           PySelectionVariableSummary_CloseDown();
VISITPY_API PyMethodDef *  PySelectionVariableSummary_GetMethodTable(int *nMethods);
bool VISITPY_API           PySelectionVariableSummary_Check(PyObject *obj);
VISITPY_API SelectionVariableSummary *  PySelectionVariableSummary_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySelectionVariableSummary_New();
VISITPY_API PyObject *     PySelectionVariableSummary_Wrap(const SelectionVariableSummary *attr);
void VISITPY_API           PySelectionVariableSummary_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySelectionVariableSummary_SetDefaults(const SelectionVariableSummary *atts);
std::string VISITPY_API    PySelectionVariableSummary_GetLogString();
std::string VISITPY_API    PySelectionVariableSummary_ToString(const SelectionVariableSummary *, const char *, const bool=false);
VISITPY_API PyObject *     PySelectionVariableSummary_getattr(PyObject *self, char *name);
int VISITPY_API            PySelectionVariableSummary_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySelectionVariableSummary_methods[SELECTIONVARIABLESUMMARY_NMETH];

#endif

