// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LAGRANGIANATTRIBUTES_H
#define PY_LAGRANGIANATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LagrangianAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LAGRANGIANATTRIBUTES_NMETH 12
void           PyLagrangianAttributes_StartUp(LagrangianAttributes *subj, void *data);
void           PyLagrangianAttributes_CloseDown();
PyMethodDef *  PyLagrangianAttributes_GetMethodTable(int *nMethods);
bool           PyLagrangianAttributes_Check(PyObject *obj);
LagrangianAttributes *  PyLagrangianAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLagrangianAttributes_New();
PyObject *     PyLagrangianAttributes_Wrap(const LagrangianAttributes *attr);
void           PyLagrangianAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLagrangianAttributes_SetDefaults(const LagrangianAttributes *atts);
std::string    PyLagrangianAttributes_GetLogString();
std::string    PyLagrangianAttributes_ToString(const LagrangianAttributes *, const char *, const bool=false);
PyObject *     PyLagrangianAttributes_getattr(PyObject *self, char *name);
int            PyLagrangianAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLagrangianAttributes_methods[LAGRANGIANATTRIBUTES_NMETH];

#endif

