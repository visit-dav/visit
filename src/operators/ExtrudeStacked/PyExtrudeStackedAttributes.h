// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXTRUDESTACKEDATTRIBUTES_H
#define PY_EXTRUDESTACKEDATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExtrudeStackedAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EXTRUDESTACKEDATTRIBUTES_NMETH 22
void           PyExtrudeStackedAttributes_StartUp(ExtrudeStackedAttributes *subj, void *data);
void           PyExtrudeStackedAttributes_CloseDown();
PyMethodDef *  PyExtrudeStackedAttributes_GetMethodTable(int *nMethods);
bool           PyExtrudeStackedAttributes_Check(PyObject *obj);
ExtrudeStackedAttributes *  PyExtrudeStackedAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExtrudeStackedAttributes_New();
PyObject *     PyExtrudeStackedAttributes_Wrap(const ExtrudeStackedAttributes *attr);
void           PyExtrudeStackedAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExtrudeStackedAttributes_SetDefaults(const ExtrudeStackedAttributes *atts);
std::string    PyExtrudeStackedAttributes_GetLogString();
std::string    PyExtrudeStackedAttributes_ToString(const ExtrudeStackedAttributes *, const char *, const bool=false);
PyObject *     PyExtrudeStackedAttributes_getattr(PyObject *self, char *name);
int            PyExtrudeStackedAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyExtrudeStackedAttributes_methods[EXTRUDESTACKEDATTRIBUTES_NMETH];

#endif

