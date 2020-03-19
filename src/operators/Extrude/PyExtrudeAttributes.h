// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXTRUDEATTRIBUTES_H
#define PY_EXTRUDEATTRIBUTES_H
#include <Python.h>
#include <ExtrudeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EXTRUDEATTRIBUTES_NMETH 22
void           PyExtrudeAttributes_StartUp(ExtrudeAttributes *subj, void *data);
void           PyExtrudeAttributes_CloseDown();
PyMethodDef *  PyExtrudeAttributes_GetMethodTable(int *nMethods);
bool           PyExtrudeAttributes_Check(PyObject *obj);
ExtrudeAttributes *  PyExtrudeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExtrudeAttributes_New();
PyObject *     PyExtrudeAttributes_Wrap(const ExtrudeAttributes *attr);
void           PyExtrudeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExtrudeAttributes_SetDefaults(const ExtrudeAttributes *atts);
std::string    PyExtrudeAttributes_GetLogString();
std::string    PyExtrudeAttributes_ToString(const ExtrudeAttributes *, const char *);
PyObject *     PyExtrudeAttributes_getattr(PyObject *self, char *name);
int            PyExtrudeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyExtrudeAttributes_methods[EXTRUDEATTRIBUTES_NMETH];

#endif

