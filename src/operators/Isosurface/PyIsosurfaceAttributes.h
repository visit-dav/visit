// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ISOSURFACEATTRIBUTES_H
#define PY_ISOSURFACEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <IsosurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ISOSURFACEATTRIBUTES_NMETH 22
void           PyIsosurfaceAttributes_StartUp(IsosurfaceAttributes *subj, void *data);
void           PyIsosurfaceAttributes_CloseDown();
PyMethodDef *  PyIsosurfaceAttributes_GetMethodTable(int *nMethods);
bool           PyIsosurfaceAttributes_Check(PyObject *obj);
IsosurfaceAttributes *  PyIsosurfaceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyIsosurfaceAttributes_New();
PyObject *     PyIsosurfaceAttributes_Wrap(const IsosurfaceAttributes *attr);
void           PyIsosurfaceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyIsosurfaceAttributes_SetDefaults(const IsosurfaceAttributes *atts);
std::string    PyIsosurfaceAttributes_GetLogString();
std::string    PyIsosurfaceAttributes_ToString(const IsosurfaceAttributes *, const char *, const bool=false);
PyObject *     PyIsosurfaceAttributes_getattr(PyObject *self, char *name);
int            PyIsosurfaceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyIsosurfaceAttributes_methods[ISOSURFACEATTRIBUTES_NMETH];

#endif

