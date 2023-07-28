// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CARTOGRAPHICPROJECTIONATTRIBUTES_H
#define PY_CARTOGRAPHICPROJECTIONATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CartographicProjectionAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CARTOGRAPHICPROJECTIONATTRIBUTES_NMETH 6
void           PyCartographicProjectionAttributes_StartUp(CartographicProjectionAttributes *subj, void *data);
void           PyCartographicProjectionAttributes_CloseDown();
PyMethodDef *  PyCartographicProjectionAttributes_GetMethodTable(int *nMethods);
bool           PyCartographicProjectionAttributes_Check(PyObject *obj);
CartographicProjectionAttributes *  PyCartographicProjectionAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCartographicProjectionAttributes_New();
PyObject *     PyCartographicProjectionAttributes_Wrap(const CartographicProjectionAttributes *attr);
void           PyCartographicProjectionAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCartographicProjectionAttributes_SetDefaults(const CartographicProjectionAttributes *atts);
std::string    PyCartographicProjectionAttributes_GetLogString();
std::string    PyCartographicProjectionAttributes_ToString(const CartographicProjectionAttributes *, const char *, const bool=false);
PyObject *     PyCartographicProjectionAttributes_getattr(PyObject *self, char *name);
int            PyCartographicProjectionAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCartographicProjectionAttributes_methods[CARTOGRAPHICPROJECTIONATTRIBUTES_NMETH];

#endif

