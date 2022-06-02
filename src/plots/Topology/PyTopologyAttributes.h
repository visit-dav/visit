// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TOPOLOGYATTRIBUTES_H
#define PY_TOPOLOGYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TopologyAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TOPOLOGYATTRIBUTES_NMETH 18
void           PyTopologyAttributes_StartUp(TopologyAttributes *subj, void *data);
void           PyTopologyAttributes_CloseDown();
PyMethodDef *  PyTopologyAttributes_GetMethodTable(int *nMethods);
bool           PyTopologyAttributes_Check(PyObject *obj);
TopologyAttributes *  PyTopologyAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTopologyAttributes_New();
PyObject *     PyTopologyAttributes_Wrap(const TopologyAttributes *attr);
void           PyTopologyAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTopologyAttributes_SetDefaults(const TopologyAttributes *atts);
std::string    PyTopologyAttributes_GetLogString();
std::string    PyTopologyAttributes_ToString(const TopologyAttributes *, const char *, const bool=false);
PyObject *     PyTopologyAttributes_getattr(PyObject *self, char *name);
int            PyTopologyAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTopologyAttributes_methods[TOPOLOGYATTRIBUTES_NMETH];

#endif

