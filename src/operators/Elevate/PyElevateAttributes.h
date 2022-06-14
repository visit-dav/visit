// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ELEVATEATTRIBUTES_H
#define PY_ELEVATEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ElevateAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ELEVATEATTRIBUTES_NMETH 22
void           PyElevateAttributes_StartUp(ElevateAttributes *subj, void *data);
void           PyElevateAttributes_CloseDown();
PyMethodDef *  PyElevateAttributes_GetMethodTable(int *nMethods);
bool           PyElevateAttributes_Check(PyObject *obj);
ElevateAttributes *  PyElevateAttributes_FromPyObject(PyObject *obj);
PyObject *     PyElevateAttributes_New();
PyObject *     PyElevateAttributes_Wrap(const ElevateAttributes *attr);
void           PyElevateAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyElevateAttributes_SetDefaults(const ElevateAttributes *atts);
std::string    PyElevateAttributes_GetLogString();
std::string    PyElevateAttributes_ToString(const ElevateAttributes *, const char *, const bool=false);
PyObject *     PyElevateAttributes_getattr(PyObject *self, char *name);
int            PyElevateAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyElevateAttributes_methods[ELEVATEATTRIBUTES_NMETH];

#endif

