// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MERGEOPERATORATTRIBUTES_H
#define PY_MERGEOPERATORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MergeOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define MERGEOPERATORATTRIBUTES_NMETH 6
void           PyMergeOperatorAttributes_StartUp(MergeOperatorAttributes *subj, void *data);
void           PyMergeOperatorAttributes_CloseDown();
PyMethodDef *  PyMergeOperatorAttributes_GetMethodTable(int *nMethods);
bool           PyMergeOperatorAttributes_Check(PyObject *obj);
MergeOperatorAttributes *  PyMergeOperatorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMergeOperatorAttributes_New();
PyObject *     PyMergeOperatorAttributes_Wrap(const MergeOperatorAttributes *attr);
void           PyMergeOperatorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMergeOperatorAttributes_SetDefaults(const MergeOperatorAttributes *atts);
std::string    PyMergeOperatorAttributes_GetLogString();
std::string    PyMergeOperatorAttributes_ToString(const MergeOperatorAttributes *, const char *, const bool=false);
PyObject *     PyMergeOperatorAttributes_getattr(PyObject *self, char *name);
int            PyMergeOperatorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMergeOperatorAttributes_methods[MERGEOPERATORATTRIBUTES_NMETH];

#endif

