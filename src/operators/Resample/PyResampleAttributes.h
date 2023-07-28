// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_RESAMPLEATTRIBUTES_H
#define PY_RESAMPLEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ResampleAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define RESAMPLEATTRIBUTES_NMETH 34
void           PyResampleAttributes_StartUp(ResampleAttributes *subj, void *data);
void           PyResampleAttributes_CloseDown();
PyMethodDef *  PyResampleAttributes_GetMethodTable(int *nMethods);
bool           PyResampleAttributes_Check(PyObject *obj);
ResampleAttributes *  PyResampleAttributes_FromPyObject(PyObject *obj);
PyObject *     PyResampleAttributes_New();
PyObject *     PyResampleAttributes_Wrap(const ResampleAttributes *attr);
void           PyResampleAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyResampleAttributes_SetDefaults(const ResampleAttributes *atts);
std::string    PyResampleAttributes_GetLogString();
std::string    PyResampleAttributes_ToString(const ResampleAttributes *, const char *, const bool=false);
PyObject *     PyResampleAttributes_getattr(PyObject *self, char *name);
int            PyResampleAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyResampleAttributes_methods[RESAMPLEATTRIBUTES_NMETH];

#endif

