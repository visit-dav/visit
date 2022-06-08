// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ONIONPEELATTRIBUTES_H
#define PY_ONIONPEELATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <OnionPeelAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ONIONPEELATTRIBUTES_NMETH 20
void           PyOnionPeelAttributes_StartUp(OnionPeelAttributes *subj, void *data);
void           PyOnionPeelAttributes_CloseDown();
PyMethodDef *  PyOnionPeelAttributes_GetMethodTable(int *nMethods);
bool           PyOnionPeelAttributes_Check(PyObject *obj);
OnionPeelAttributes *  PyOnionPeelAttributes_FromPyObject(PyObject *obj);
PyObject *     PyOnionPeelAttributes_New();
PyObject *     PyOnionPeelAttributes_Wrap(const OnionPeelAttributes *attr);
void           PyOnionPeelAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyOnionPeelAttributes_SetDefaults(const OnionPeelAttributes *atts);
std::string    PyOnionPeelAttributes_GetLogString();
std::string    PyOnionPeelAttributes_ToString(const OnionPeelAttributes *, const char *, const bool=false);
PyObject *     PyOnionPeelAttributes_getattr(PyObject *self, char *name);
int            PyOnionPeelAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyOnionPeelAttributes_methods[ONIONPEELATTRIBUTES_NMETH];

#endif

