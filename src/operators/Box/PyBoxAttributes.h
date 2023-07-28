// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_BOXATTRIBUTES_H
#define PY_BOXATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <BoxAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define BOXATTRIBUTES_NMETH 18
void           PyBoxAttributes_StartUp(BoxAttributes *subj, void *data);
void           PyBoxAttributes_CloseDown();
PyMethodDef *  PyBoxAttributes_GetMethodTable(int *nMethods);
bool           PyBoxAttributes_Check(PyObject *obj);
BoxAttributes *  PyBoxAttributes_FromPyObject(PyObject *obj);
PyObject *     PyBoxAttributes_New();
PyObject *     PyBoxAttributes_Wrap(const BoxAttributes *attr);
void           PyBoxAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyBoxAttributes_SetDefaults(const BoxAttributes *atts);
std::string    PyBoxAttributes_GetLogString();
std::string    PyBoxAttributes_ToString(const BoxAttributes *, const char *, const bool=false);
PyObject *     PyBoxAttributes_getattr(PyObject *self, char *name);
int            PyBoxAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyBoxAttributes_methods[BOXATTRIBUTES_NMETH];

#endif

