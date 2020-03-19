// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LINESURFACEATTRIBUTES_H
#define PY_LINESURFACEATTRIBUTES_H
#include <Python.h>
#include <LineSurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LINESURFACEATTRIBUTES_NMETH 12
void           PyLineSurfaceAttributes_StartUp(LineSurfaceAttributes *subj, void *data);
void           PyLineSurfaceAttributes_CloseDown();
PyMethodDef *  PyLineSurfaceAttributes_GetMethodTable(int *nMethods);
bool           PyLineSurfaceAttributes_Check(PyObject *obj);
LineSurfaceAttributes *  PyLineSurfaceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLineSurfaceAttributes_New();
PyObject *     PyLineSurfaceAttributes_Wrap(const LineSurfaceAttributes *attr);
void           PyLineSurfaceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLineSurfaceAttributes_SetDefaults(const LineSurfaceAttributes *atts);
std::string    PyLineSurfaceAttributes_GetLogString();
std::string    PyLineSurfaceAttributes_ToString(const LineSurfaceAttributes *, const char *);
PyObject *     PyLineSurfaceAttributes_getattr(PyObject *self, char *name);
int            PyLineSurfaceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLineSurfaceAttributes_methods[LINESURFACEATTRIBUTES_NMETH];

#endif

