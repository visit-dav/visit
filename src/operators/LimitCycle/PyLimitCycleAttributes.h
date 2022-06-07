// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LIMITCYCLEATTRIBUTES_H
#define PY_LIMITCYCLEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LimitCycleAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LIMITCYCLEATTRIBUTES_NMETH 120
void           PyLimitCycleAttributes_StartUp(LimitCycleAttributes *subj, void *data);
void           PyLimitCycleAttributes_CloseDown();
PyMethodDef *  PyLimitCycleAttributes_GetMethodTable(int *nMethods);
bool           PyLimitCycleAttributes_Check(PyObject *obj);
LimitCycleAttributes *  PyLimitCycleAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLimitCycleAttributes_New();
PyObject *     PyLimitCycleAttributes_Wrap(const LimitCycleAttributes *attr);
void           PyLimitCycleAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLimitCycleAttributes_SetDefaults(const LimitCycleAttributes *atts);
std::string    PyLimitCycleAttributes_GetLogString();
std::string    PyLimitCycleAttributes_ToString(const LimitCycleAttributes *, const char *, const bool=false);
PyObject *     PyLimitCycleAttributes_getattr(PyObject *self, char *name);
int            PyLimitCycleAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLimitCycleAttributes_methods[LIMITCYCLEATTRIBUTES_NMETH];

#endif

