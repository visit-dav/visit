// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TRANSFORMATTRIBUTES_H
#define PY_TRANSFORMATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TransformAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TRANSFORMATTRIBUTES_NMETH 76
void           PyTransformAttributes_StartUp(TransformAttributes *subj, void *data);
void           PyTransformAttributes_CloseDown();
PyMethodDef *  PyTransformAttributes_GetMethodTable(int *nMethods);
bool           PyTransformAttributes_Check(PyObject *obj);
TransformAttributes *  PyTransformAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTransformAttributes_New();
PyObject *     PyTransformAttributes_Wrap(const TransformAttributes *attr);
void           PyTransformAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTransformAttributes_SetDefaults(const TransformAttributes *atts);
std::string    PyTransformAttributes_GetLogString();
std::string    PyTransformAttributes_ToString(const TransformAttributes *, const char *, const bool=false);
PyObject *     PyTransformAttributes_getattr(PyObject *self, char *name);
int            PyTransformAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTransformAttributes_methods[TRANSFORMATTRIBUTES_NMETH];

#endif

