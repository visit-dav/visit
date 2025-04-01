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

#endif

