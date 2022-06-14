// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SELECTIONLIST_H
#define PY_SELECTIONLIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SelectionList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SELECTIONLIST_NMETH 14
void VISITPY_API           PySelectionList_StartUp(SelectionList *subj, void *data);
void VISITPY_API           PySelectionList_CloseDown();
VISITPY_API PyMethodDef *  PySelectionList_GetMethodTable(int *nMethods);
bool VISITPY_API           PySelectionList_Check(PyObject *obj);
VISITPY_API SelectionList *  PySelectionList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySelectionList_New();
VISITPY_API PyObject *     PySelectionList_Wrap(const SelectionList *attr);
void VISITPY_API           PySelectionList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySelectionList_SetDefaults(const SelectionList *atts);
std::string VISITPY_API    PySelectionList_GetLogString();
std::string VISITPY_API    PySelectionList_ToString(const SelectionList *, const char *, const bool=false);
VISITPY_API PyObject *     PySelectionList_getattr(PyObject *self, char *name);
int VISITPY_API            PySelectionList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySelectionList_methods[SELECTIONLIST_NMETH];

#endif

