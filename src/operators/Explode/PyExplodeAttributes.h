// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXPLODEATTRIBUTES_H
#define PY_EXPLODEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExplodeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EXPLODEATTRIBUTES_NMETH 35
void           PyExplodeAttributes_StartUp(ExplodeAttributes *subj, void *data);
void           PyExplodeAttributes_CloseDown();
PyMethodDef *  PyExplodeAttributes_GetMethodTable(int *nMethods);
bool           PyExplodeAttributes_Check(PyObject *obj);
ExplodeAttributes *  PyExplodeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExplodeAttributes_New();
PyObject *     PyExplodeAttributes_Wrap(const ExplodeAttributes *attr);
void           PyExplodeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExplodeAttributes_SetDefaults(const ExplodeAttributes *atts);
std::string    PyExplodeAttributes_GetLogString();
std::string    PyExplodeAttributes_ToString(const ExplodeAttributes *, const char *, const bool=false);
PyObject *     PyExplodeAttributes_getattr(PyObject *self, char *name);
int            PyExplodeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyExplodeAttributes_methods[EXPLODEATTRIBUTES_NMETH];

#endif

