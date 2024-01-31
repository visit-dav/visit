// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EDGEATTRIBUTES_H
#define PY_EDGEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <EdgeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EDGEATTRIBUTES_NMETH 4
void           PyEdgeAttributes_StartUp(EdgeAttributes *subj, void *data);
void           PyEdgeAttributes_CloseDown();
PyMethodDef *  PyEdgeAttributes_GetMethodTable(int *nMethods);
bool           PyEdgeAttributes_Check(PyObject *obj);
EdgeAttributes *  PyEdgeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyEdgeAttributes_New();
PyObject *     PyEdgeAttributes_Wrap(const EdgeAttributes *attr);
void           PyEdgeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyEdgeAttributes_SetDefaults(const EdgeAttributes *atts);
std::string    PyEdgeAttributes_GetLogString();
std::string    PyEdgeAttributes_ToString(const EdgeAttributes *, const char *, const bool=false);
PyObject *     PyEdgeAttributes_getattr(PyObject *self, char *name);
int            PyEdgeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyEdgeAttributes_methods[EDGEATTRIBUTES_NMETH];

#endif

