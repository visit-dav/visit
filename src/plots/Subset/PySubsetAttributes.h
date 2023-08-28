// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SUBSETATTRIBUTES_H
#define PY_SUBSETATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SubsetAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SUBSETATTRIBUTES_NMETH 36
void           PySubsetAttributes_StartUp(SubsetAttributes *subj, void *data);
void           PySubsetAttributes_CloseDown();
PyMethodDef *  PySubsetAttributes_GetMethodTable(int *nMethods);
bool           PySubsetAttributes_Check(PyObject *obj);
SubsetAttributes *  PySubsetAttributes_FromPyObject(PyObject *obj);
PyObject *     PySubsetAttributes_New();
PyObject *     PySubsetAttributes_Wrap(const SubsetAttributes *attr);
void           PySubsetAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySubsetAttributes_SetDefaults(const SubsetAttributes *atts);
std::string    PySubsetAttributes_GetLogString();
std::string    PySubsetAttributes_ToString(const SubsetAttributes *, const char *, const bool=false);
PyObject *     PySubsetAttributes_getattr(PyObject *self, char *name);
int            PySubsetAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySubsetAttributes_methods[SUBSETATTRIBUTES_NMETH];

#endif

