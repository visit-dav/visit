// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ELLIPSOIDSLICEATTRIBUTES_H
#define PY_ELLIPSOIDSLICEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <EllipsoidSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ELLIPSOIDSLICEATTRIBUTES_NMETH 8
void           PyEllipsoidSliceAttributes_StartUp(EllipsoidSliceAttributes *subj, void *data);
void           PyEllipsoidSliceAttributes_CloseDown();
PyMethodDef *  PyEllipsoidSliceAttributes_GetMethodTable(int *nMethods);
bool           PyEllipsoidSliceAttributes_Check(PyObject *obj);
EllipsoidSliceAttributes *  PyEllipsoidSliceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyEllipsoidSliceAttributes_New();
PyObject *     PyEllipsoidSliceAttributes_Wrap(const EllipsoidSliceAttributes *attr);
void           PyEllipsoidSliceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyEllipsoidSliceAttributes_SetDefaults(const EllipsoidSliceAttributes *atts);
std::string    PyEllipsoidSliceAttributes_GetLogString();
std::string    PyEllipsoidSliceAttributes_ToString(const EllipsoidSliceAttributes *, const char *, const bool=false);
PyObject *     PyEllipsoidSliceAttributes_getattr(PyObject *self, char *name);
int            PyEllipsoidSliceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyEllipsoidSliceAttributes_methods[ELLIPSOIDSLICEATTRIBUTES_NMETH];

#endif

