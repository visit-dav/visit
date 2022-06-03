// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DISPLACEATTRIBUTES_H
#define PY_DISPLACEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DisplaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DISPLACEATTRIBUTES_NMETH 6
void           PyDisplaceAttributes_StartUp(DisplaceAttributes *subj, void *data);
void           PyDisplaceAttributes_CloseDown();
PyMethodDef *  PyDisplaceAttributes_GetMethodTable(int *nMethods);
bool           PyDisplaceAttributes_Check(PyObject *obj);
DisplaceAttributes *  PyDisplaceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDisplaceAttributes_New();
PyObject *     PyDisplaceAttributes_Wrap(const DisplaceAttributes *attr);
void           PyDisplaceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDisplaceAttributes_SetDefaults(const DisplaceAttributes *atts);
std::string    PyDisplaceAttributes_GetLogString();
std::string    PyDisplaceAttributes_ToString(const DisplaceAttributes *, const char *, const bool=false);
PyObject *     PyDisplaceAttributes_getattr(PyObject *self, char *name);
int            PyDisplaceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDisplaceAttributes_methods[DISPLACEATTRIBUTES_NMETH];

#endif

