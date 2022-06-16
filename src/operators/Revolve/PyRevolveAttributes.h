// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_REVOLVEATTRIBUTES_H
#define PY_REVOLVEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <RevolveAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define REVOLVEATTRIBUTES_NMETH 14
void           PyRevolveAttributes_StartUp(RevolveAttributes *subj, void *data);
void           PyRevolveAttributes_CloseDown();
PyMethodDef *  PyRevolveAttributes_GetMethodTable(int *nMethods);
bool           PyRevolveAttributes_Check(PyObject *obj);
RevolveAttributes *  PyRevolveAttributes_FromPyObject(PyObject *obj);
PyObject *     PyRevolveAttributes_New();
PyObject *     PyRevolveAttributes_Wrap(const RevolveAttributes *attr);
void           PyRevolveAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyRevolveAttributes_SetDefaults(const RevolveAttributes *atts);
std::string    PyRevolveAttributes_GetLogString();
std::string    PyRevolveAttributes_ToString(const RevolveAttributes *, const char *, const bool=false);
PyObject *     PyRevolveAttributes_getattr(PyObject *self, char *name);
int            PyRevolveAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyRevolveAttributes_methods[REVOLVEATTRIBUTES_NMETH];

#endif

