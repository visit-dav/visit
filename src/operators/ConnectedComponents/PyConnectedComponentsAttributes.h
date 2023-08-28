// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONNECTEDCOMPONENTSATTRIBUTES_H
#define PY_CONNECTEDCOMPONENTSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ConnectedComponentsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CONNECTEDCOMPONENTSATTRIBUTES_NMETH 4
void           PyConnectedComponentsAttributes_StartUp(ConnectedComponentsAttributes *subj, void *data);
void           PyConnectedComponentsAttributes_CloseDown();
PyMethodDef *  PyConnectedComponentsAttributes_GetMethodTable(int *nMethods);
bool           PyConnectedComponentsAttributes_Check(PyObject *obj);
ConnectedComponentsAttributes *  PyConnectedComponentsAttributes_FromPyObject(PyObject *obj);
PyObject *     PyConnectedComponentsAttributes_New();
PyObject *     PyConnectedComponentsAttributes_Wrap(const ConnectedComponentsAttributes *attr);
void           PyConnectedComponentsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyConnectedComponentsAttributes_SetDefaults(const ConnectedComponentsAttributes *atts);
std::string    PyConnectedComponentsAttributes_GetLogString();
std::string    PyConnectedComponentsAttributes_ToString(const ConnectedComponentsAttributes *, const char *, const bool=false);
PyObject *     PyConnectedComponentsAttributes_getattr(PyObject *self, char *name);
int            PyConnectedComponentsAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyConnectedComponentsAttributes_methods[CONNECTEDCOMPONENTSATTRIBUTES_NMETH];

#endif

