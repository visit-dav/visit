// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LINESAMPLERATTRIBUTES_H
#define PY_LINESAMPLERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LineSamplerAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LINESAMPLERATTRIBUTES_NMETH 106
void           PyLineSamplerAttributes_StartUp(LineSamplerAttributes *subj, void *data);
void           PyLineSamplerAttributes_CloseDown();
PyMethodDef *  PyLineSamplerAttributes_GetMethodTable(int *nMethods);
bool           PyLineSamplerAttributes_Check(PyObject *obj);
LineSamplerAttributes *  PyLineSamplerAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLineSamplerAttributes_New();
PyObject *     PyLineSamplerAttributes_Wrap(const LineSamplerAttributes *attr);
void           PyLineSamplerAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLineSamplerAttributes_SetDefaults(const LineSamplerAttributes *atts);
std::string    PyLineSamplerAttributes_GetLogString();
std::string    PyLineSamplerAttributes_ToString(const LineSamplerAttributes *, const char *, const bool=false);
PyObject *     PyLineSamplerAttributes_getattr(PyObject *self, char *name);
int            PyLineSamplerAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLineSamplerAttributes_methods[LINESAMPLERATTRIBUTES_NMETH];

#endif

