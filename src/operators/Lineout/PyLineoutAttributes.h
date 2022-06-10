// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LINEOUTATTRIBUTES_H
#define PY_LINEOUTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LineoutAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LINEOUTATTRIBUTES_NMETH 16
void           PyLineoutAttributes_StartUp(LineoutAttributes *subj, void *data);
void           PyLineoutAttributes_CloseDown();
PyMethodDef *  PyLineoutAttributes_GetMethodTable(int *nMethods);
bool           PyLineoutAttributes_Check(PyObject *obj);
LineoutAttributes *  PyLineoutAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLineoutAttributes_New();
PyObject *     PyLineoutAttributes_Wrap(const LineoutAttributes *attr);
void           PyLineoutAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLineoutAttributes_SetDefaults(const LineoutAttributes *atts);
std::string    PyLineoutAttributes_GetLogString();
std::string    PyLineoutAttributes_ToString(const LineoutAttributes *, const char *, const bool=false);
PyObject *     PyLineoutAttributes_getattr(PyObject *self, char *name);
int            PyLineoutAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLineoutAttributes_methods[LINEOUTATTRIBUTES_NMETH];

#endif

