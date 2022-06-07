// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FLUXATTRIBUTES_H
#define PY_FLUXATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FluxAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define FLUXATTRIBUTES_NMETH 8
void           PyFluxAttributes_StartUp(FluxAttributes *subj, void *data);
void           PyFluxAttributes_CloseDown();
PyMethodDef *  PyFluxAttributes_GetMethodTable(int *nMethods);
bool           PyFluxAttributes_Check(PyObject *obj);
FluxAttributes *  PyFluxAttributes_FromPyObject(PyObject *obj);
PyObject *     PyFluxAttributes_New();
PyObject *     PyFluxAttributes_Wrap(const FluxAttributes *attr);
void           PyFluxAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyFluxAttributes_SetDefaults(const FluxAttributes *atts);
std::string    PyFluxAttributes_GetLogString();
std::string    PyFluxAttributes_ToString(const FluxAttributes *, const char *, const bool=false);
PyObject *     PyFluxAttributes_getattr(PyObject *self, char *name);
int            PyFluxAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyFluxAttributes_methods[FLUXATTRIBUTES_NMETH];

#endif

