// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SCATTERATTRIBUTES_H
#define PY_SCATTERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ScatterAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SCATTERATTRIBUTES_NMETH 84
void           PyScatterAttributes_StartUp(ScatterAttributes *subj, void *data);
void           PyScatterAttributes_CloseDown();
PyMethodDef *  PyScatterAttributes_GetMethodTable(int *nMethods);
bool           PyScatterAttributes_Check(PyObject *obj);
ScatterAttributes *  PyScatterAttributes_FromPyObject(PyObject *obj);
PyObject *     PyScatterAttributes_New();
PyObject *     PyScatterAttributes_Wrap(const ScatterAttributes *attr);
void           PyScatterAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyScatterAttributes_SetDefaults(const ScatterAttributes *atts);
std::string    PyScatterAttributes_GetLogString();
std::string    PyScatterAttributes_ToString(const ScatterAttributes *, const char *, const bool=false);
PyObject *     PyScatterAttributes_getattr(PyObject *self, char *name);
int            PyScatterAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyScatterAttributes_methods[SCATTERATTRIBUTES_NMETH];

#endif

