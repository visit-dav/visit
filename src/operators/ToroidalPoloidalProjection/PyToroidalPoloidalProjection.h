// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TOROIDALPOLOIDALPROJECTION_H
#define PY_TOROIDALPOLOIDALPROJECTION_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ToroidalPoloidalProjection.h>

//
// Functions exposed to the VisIt module.
//
#define TOROIDALPOLOIDALPROJECTION_NMETH 12
void           PyToroidalPoloidalProjection_StartUp(ToroidalPoloidalProjection *subj, void *data);
void           PyToroidalPoloidalProjection_CloseDown();
PyMethodDef *  PyToroidalPoloidalProjection_GetMethodTable(int *nMethods);
bool           PyToroidalPoloidalProjection_Check(PyObject *obj);
ToroidalPoloidalProjection *  PyToroidalPoloidalProjection_FromPyObject(PyObject *obj);
PyObject *     PyToroidalPoloidalProjection_New();
PyObject *     PyToroidalPoloidalProjection_Wrap(const ToroidalPoloidalProjection *attr);
void           PyToroidalPoloidalProjection_SetParent(PyObject *obj, PyObject *parent);
void           PyToroidalPoloidalProjection_SetDefaults(const ToroidalPoloidalProjection *atts);
std::string    PyToroidalPoloidalProjection_GetLogString();
std::string    PyToroidalPoloidalProjection_ToString(const ToroidalPoloidalProjection *, const char *, const bool=false);
PyObject *     PyToroidalPoloidalProjection_getattr(PyObject *self, char *name);
int            PyToroidalPoloidalProjection_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyToroidalPoloidalProjection_methods[TOROIDALPOLOIDALPROJECTION_NMETH];

#endif

