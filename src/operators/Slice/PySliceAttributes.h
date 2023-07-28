// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SLICEATTRIBUTES_H
#define PY_SLICEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SLICEATTRIBUTES_NMETH 36
void           PySliceAttributes_StartUp(SliceAttributes *subj, void *data);
void           PySliceAttributes_CloseDown();
PyMethodDef *  PySliceAttributes_GetMethodTable(int *nMethods);
bool           PySliceAttributes_Check(PyObject *obj);
SliceAttributes *  PySliceAttributes_FromPyObject(PyObject *obj);
PyObject *     PySliceAttributes_New();
PyObject *     PySliceAttributes_Wrap(const SliceAttributes *attr);
void           PySliceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySliceAttributes_SetDefaults(const SliceAttributes *atts);
std::string    PySliceAttributes_GetLogString();
std::string    PySliceAttributes_ToString(const SliceAttributes *, const char *, const bool=false);
PyObject *     PySliceAttributes_getattr(PyObject *self, char *name);
int            PySliceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySliceAttributes_methods[SLICEATTRIBUTES_NMETH];

#endif

