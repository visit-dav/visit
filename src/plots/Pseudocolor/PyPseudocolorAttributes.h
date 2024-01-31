// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PSEUDOCOLORATTRIBUTES_H
#define PY_PSEUDOCOLORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PseudocolorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PSEUDOCOLORATTRIBUTES_NMETH 108
void           PyPseudocolorAttributes_StartUp(PseudocolorAttributes *subj, void *data);
void           PyPseudocolorAttributes_CloseDown();
PyMethodDef *  PyPseudocolorAttributes_GetMethodTable(int *nMethods);
bool           PyPseudocolorAttributes_Check(PyObject *obj);
PseudocolorAttributes *  PyPseudocolorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyPseudocolorAttributes_New();
PyObject *     PyPseudocolorAttributes_Wrap(const PseudocolorAttributes *attr);
void           PyPseudocolorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyPseudocolorAttributes_SetDefaults(const PseudocolorAttributes *atts);
std::string    PyPseudocolorAttributes_GetLogString();
std::string    PyPseudocolorAttributes_ToString(const PseudocolorAttributes *, const char *, const bool=false);
PyObject *     PyPseudocolorAttributes_getattr(PyObject *self, char *name);
int            PyPseudocolorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyPseudocolorAttributes_methods[PSEUDOCOLORATTRIBUTES_NMETH];

#endif

