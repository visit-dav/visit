// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SURFACEATTRIBUTES_H
#define PY_SURFACEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PySurfaceAttributes_StartUp(SurfaceAttributes *subj, void *data);
void           PySurfaceAttributes_CloseDown();
PyMethodDef *  PySurfaceAttributes_GetMethodTable(int *nMethods);
bool           PySurfaceAttributes_Check(PyObject *obj);
SurfaceAttributes *  PySurfaceAttributes_FromPyObject(PyObject *obj);
PyObject *     PySurfaceAttributes_New();
PyObject *     PySurfaceAttributes_Wrap(const SurfaceAttributes *attr);
void           PySurfaceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySurfaceAttributes_SetDefaults(const SurfaceAttributes *atts);
std::string    PySurfaceAttributes_GetLogString();
std::string    PySurfaceAttributes_ToString(const SurfaceAttributes *, const char *, const bool=false);

#endif

