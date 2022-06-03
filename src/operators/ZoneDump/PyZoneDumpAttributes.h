// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ZONEDUMPATTRIBUTES_H
#define PY_ZONEDUMPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ZoneDumpAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ZONEDUMPATTRIBUTES_NMETH 12
void           PyZoneDumpAttributes_StartUp(ZoneDumpAttributes *subj, void *data);
void           PyZoneDumpAttributes_CloseDown();
PyMethodDef *  PyZoneDumpAttributes_GetMethodTable(int *nMethods);
bool           PyZoneDumpAttributes_Check(PyObject *obj);
ZoneDumpAttributes *  PyZoneDumpAttributes_FromPyObject(PyObject *obj);
PyObject *     PyZoneDumpAttributes_New();
PyObject *     PyZoneDumpAttributes_Wrap(const ZoneDumpAttributes *attr);
void           PyZoneDumpAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyZoneDumpAttributes_SetDefaults(const ZoneDumpAttributes *atts);
std::string    PyZoneDumpAttributes_GetLogString();
std::string    PyZoneDumpAttributes_ToString(const ZoneDumpAttributes *, const char *, const bool=false);
PyObject *     PyZoneDumpAttributes_getattr(PyObject *self, char *name);
int            PyZoneDumpAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyZoneDumpAttributes_methods[ZONEDUMPATTRIBUTES_NMETH];

#endif

