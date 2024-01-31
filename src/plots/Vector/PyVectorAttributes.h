// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VECTORATTRIBUTES_H
#define PY_VECTORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <VectorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define VECTORATTRIBUTES_NMETH 56
void           PyVectorAttributes_StartUp(VectorAttributes *subj, void *data);
void           PyVectorAttributes_CloseDown();
PyMethodDef *  PyVectorAttributes_GetMethodTable(int *nMethods);
bool           PyVectorAttributes_Check(PyObject *obj);
VectorAttributes *  PyVectorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyVectorAttributes_New();
PyObject *     PyVectorAttributes_Wrap(const VectorAttributes *attr);
void           PyVectorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyVectorAttributes_SetDefaults(const VectorAttributes *atts);
std::string    PyVectorAttributes_GetLogString();
std::string    PyVectorAttributes_ToString(const VectorAttributes *, const char *, const bool=false);
PyObject *     PyVectorAttributes_getattr(PyObject *self, char *name);
int            PyVectorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyVectorAttributes_methods[VECTORATTRIBUTES_NMETH];

#endif

