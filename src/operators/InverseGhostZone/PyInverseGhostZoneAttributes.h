// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_INVERSEGHOSTZONEATTRIBUTES_H
#define PY_INVERSEGHOSTZONEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <InverseGhostZoneAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define INVERSEGHOSTZONEATTRIBUTES_NMETH 16
void           PyInverseGhostZoneAttributes_StartUp(InverseGhostZoneAttributes *subj, void *data);
void           PyInverseGhostZoneAttributes_CloseDown();
PyMethodDef *  PyInverseGhostZoneAttributes_GetMethodTable(int *nMethods);
bool           PyInverseGhostZoneAttributes_Check(PyObject *obj);
InverseGhostZoneAttributes *  PyInverseGhostZoneAttributes_FromPyObject(PyObject *obj);
PyObject *     PyInverseGhostZoneAttributes_New();
PyObject *     PyInverseGhostZoneAttributes_Wrap(const InverseGhostZoneAttributes *attr);
void           PyInverseGhostZoneAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyInverseGhostZoneAttributes_SetDefaults(const InverseGhostZoneAttributes *atts);
std::string    PyInverseGhostZoneAttributes_GetLogString();
std::string    PyInverseGhostZoneAttributes_ToString(const InverseGhostZoneAttributes *, const char *, const bool=false);
PyObject *     PyInverseGhostZoneAttributes_getattr(PyObject *self, char *name);
int            PyInverseGhostZoneAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyInverseGhostZoneAttributes_methods[INVERSEGHOSTZONEATTRIBUTES_NMETH];

#endif

