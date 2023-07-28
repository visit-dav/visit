// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_THREESLICEATTRIBUTES_H
#define PY_THREESLICEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ThreeSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define THREESLICEATTRIBUTES_NMETH 10
void           PyThreeSliceAttributes_StartUp(ThreeSliceAttributes *subj, void *data);
void           PyThreeSliceAttributes_CloseDown();
PyMethodDef *  PyThreeSliceAttributes_GetMethodTable(int *nMethods);
bool           PyThreeSliceAttributes_Check(PyObject *obj);
ThreeSliceAttributes *  PyThreeSliceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyThreeSliceAttributes_New();
PyObject *     PyThreeSliceAttributes_Wrap(const ThreeSliceAttributes *attr);
void           PyThreeSliceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyThreeSliceAttributes_SetDefaults(const ThreeSliceAttributes *atts);
std::string    PyThreeSliceAttributes_GetLogString();
std::string    PyThreeSliceAttributes_ToString(const ThreeSliceAttributes *, const char *, const bool=false);
PyObject *     PyThreeSliceAttributes_getattr(PyObject *self, char *name);
int            PyThreeSliceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyThreeSliceAttributes_methods[THREESLICEATTRIBUTES_NMETH];

#endif

