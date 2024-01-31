// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_POINCAREATTRIBUTES_H
#define PY_POINCAREATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PoincareAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define POINCAREATTRIBUTES_NMETH 156
void           PyPoincareAttributes_StartUp(PoincareAttributes *subj, void *data);
void           PyPoincareAttributes_CloseDown();
PyMethodDef *  PyPoincareAttributes_GetMethodTable(int *nMethods);
bool           PyPoincareAttributes_Check(PyObject *obj);
PoincareAttributes *  PyPoincareAttributes_FromPyObject(PyObject *obj);
PyObject *     PyPoincareAttributes_New();
PyObject *     PyPoincareAttributes_Wrap(const PoincareAttributes *attr);
void           PyPoincareAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyPoincareAttributes_SetDefaults(const PoincareAttributes *atts);
std::string    PyPoincareAttributes_GetLogString();
std::string    PyPoincareAttributes_ToString(const PoincareAttributes *, const char *, const bool=false);
PyObject *     PyPoincareAttributes_getattr(PyObject *self, char *name);
int            PyPoincareAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyPoincareAttributes_methods[POINCAREATTRIBUTES_NMETH];

#endif

