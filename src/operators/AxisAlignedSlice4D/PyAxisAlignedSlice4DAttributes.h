// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXISALIGNEDSLICE4DATTRIBUTES_H
#define PY_AXISALIGNEDSLICE4DATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxisAlignedSlice4DAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define AXISALIGNEDSLICE4DATTRIBUTES_NMETH 10
void           PyAxisAlignedSlice4DAttributes_StartUp(AxisAlignedSlice4DAttributes *subj, void *data);
void           PyAxisAlignedSlice4DAttributes_CloseDown();
PyMethodDef *  PyAxisAlignedSlice4DAttributes_GetMethodTable(int *nMethods);
bool           PyAxisAlignedSlice4DAttributes_Check(PyObject *obj);
AxisAlignedSlice4DAttributes *  PyAxisAlignedSlice4DAttributes_FromPyObject(PyObject *obj);
PyObject *     PyAxisAlignedSlice4DAttributes_New();
PyObject *     PyAxisAlignedSlice4DAttributes_Wrap(const AxisAlignedSlice4DAttributes *attr);
void           PyAxisAlignedSlice4DAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyAxisAlignedSlice4DAttributes_SetDefaults(const AxisAlignedSlice4DAttributes *atts);
std::string    PyAxisAlignedSlice4DAttributes_GetLogString();
std::string    PyAxisAlignedSlice4DAttributes_ToString(const AxisAlignedSlice4DAttributes *, const char *, const bool=false);
PyObject *     PyAxisAlignedSlice4DAttributes_getattr(PyObject *self, char *name);
int            PyAxisAlignedSlice4DAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyAxisAlignedSlice4DAttributes_methods[AXISALIGNEDSLICE4DATTRIBUTES_NMETH];

#endif

