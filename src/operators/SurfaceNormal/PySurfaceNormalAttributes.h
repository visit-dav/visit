// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SURFACENORMALATTRIBUTES_H
#define PY_SURFACENORMALATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SurfaceNormalAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PySurfaceNormalAttributes_StartUp(SurfaceNormalAttributes *subj, void *data);
void           PySurfaceNormalAttributes_CloseDown();
PyMethodDef *  PySurfaceNormalAttributes_GetMethodTable(int *nMethods);
bool           PySurfaceNormalAttributes_Check(PyObject *obj);
SurfaceNormalAttributes *  PySurfaceNormalAttributes_FromPyObject(PyObject *obj);
PyObject *     PySurfaceNormalAttributes_New();
PyObject *     PySurfaceNormalAttributes_Wrap(const SurfaceNormalAttributes *attr);
void           PySurfaceNormalAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySurfaceNormalAttributes_SetDefaults(const SurfaceNormalAttributes *atts);
std::string    PySurfaceNormalAttributes_GetLogString();
std::string    PySurfaceNormalAttributes_ToString(const SurfaceNormalAttributes *, const char *, const bool=false);

#endif

