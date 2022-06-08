// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_RADIALRESAMPLEATTRIBUTES_H
#define PY_RADIALRESAMPLEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <RadialResampleAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define RADIALRESAMPLEATTRIBUTES_NMETH 24
void           PyRadialResampleAttributes_StartUp(RadialResampleAttributes *subj, void *data);
void           PyRadialResampleAttributes_CloseDown();
PyMethodDef *  PyRadialResampleAttributes_GetMethodTable(int *nMethods);
bool           PyRadialResampleAttributes_Check(PyObject *obj);
RadialResampleAttributes *  PyRadialResampleAttributes_FromPyObject(PyObject *obj);
PyObject *     PyRadialResampleAttributes_New();
PyObject *     PyRadialResampleAttributes_Wrap(const RadialResampleAttributes *attr);
void           PyRadialResampleAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyRadialResampleAttributes_SetDefaults(const RadialResampleAttributes *atts);
std::string    PyRadialResampleAttributes_GetLogString();
std::string    PyRadialResampleAttributes_ToString(const RadialResampleAttributes *, const char *, const bool=false);
PyObject *     PyRadialResampleAttributes_getattr(PyObject *self, char *name);
int            PyRadialResampleAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyRadialResampleAttributes_methods[RADIALRESAMPLEATTRIBUTES_NMETH];

#endif

