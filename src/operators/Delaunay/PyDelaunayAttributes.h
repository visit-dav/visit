// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DELAUNAYATTRIBUTES_H
#define PY_DELAUNAYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DelaunayAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DELAUNAYATTRIBUTES_NMETH 4
void           PyDelaunayAttributes_StartUp(DelaunayAttributes *subj, void *data);
void           PyDelaunayAttributes_CloseDown();
PyMethodDef *  PyDelaunayAttributes_GetMethodTable(int *nMethods);
bool           PyDelaunayAttributes_Check(PyObject *obj);
DelaunayAttributes *  PyDelaunayAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDelaunayAttributes_New();
PyObject *     PyDelaunayAttributes_Wrap(const DelaunayAttributes *attr);
void           PyDelaunayAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDelaunayAttributes_SetDefaults(const DelaunayAttributes *atts);
std::string    PyDelaunayAttributes_GetLogString();
std::string    PyDelaunayAttributes_ToString(const DelaunayAttributes *, const char *, const bool=false);
PyObject *     PyDelaunayAttributes_getattr(PyObject *self, char *name);
int            PyDelaunayAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDelaunayAttributes_methods[DELAUNAYATTRIBUTES_NMETH];

#endif

