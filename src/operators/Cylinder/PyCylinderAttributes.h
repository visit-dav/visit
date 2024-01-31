// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CYLINDERATTRIBUTES_H
#define PY_CYLINDERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CylinderAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CYLINDERATTRIBUTES_NMETH 10
void           PyCylinderAttributes_StartUp(CylinderAttributes *subj, void *data);
void           PyCylinderAttributes_CloseDown();
PyMethodDef *  PyCylinderAttributes_GetMethodTable(int *nMethods);
bool           PyCylinderAttributes_Check(PyObject *obj);
CylinderAttributes *  PyCylinderAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCylinderAttributes_New();
PyObject *     PyCylinderAttributes_Wrap(const CylinderAttributes *attr);
void           PyCylinderAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCylinderAttributes_SetDefaults(const CylinderAttributes *atts);
std::string    PyCylinderAttributes_GetLogString();
std::string    PyCylinderAttributes_ToString(const CylinderAttributes *, const char *, const bool=false);
PyObject *     PyCylinderAttributes_getattr(PyObject *self, char *name);
int            PyCylinderAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCylinderAttributes_methods[CYLINDERATTRIBUTES_NMETH];

#endif

