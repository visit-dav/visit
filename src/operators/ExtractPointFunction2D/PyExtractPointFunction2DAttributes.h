// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXTRACTPOINTFUNCTION2DATTRIBUTES_H
#define PY_EXTRACTPOINTFUNCTION2DATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExtractPointFunction2DAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EXTRACTPOINTFUNCTION2DATTRIBUTES_NMETH 6
void           PyExtractPointFunction2DAttributes_StartUp(ExtractPointFunction2DAttributes *subj, void *data);
void           PyExtractPointFunction2DAttributes_CloseDown();
PyMethodDef *  PyExtractPointFunction2DAttributes_GetMethodTable(int *nMethods);
bool           PyExtractPointFunction2DAttributes_Check(PyObject *obj);
ExtractPointFunction2DAttributes *  PyExtractPointFunction2DAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExtractPointFunction2DAttributes_New();
PyObject *     PyExtractPointFunction2DAttributes_Wrap(const ExtractPointFunction2DAttributes *attr);
void           PyExtractPointFunction2DAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExtractPointFunction2DAttributes_SetDefaults(const ExtractPointFunction2DAttributes *atts);
std::string    PyExtractPointFunction2DAttributes_GetLogString();
std::string    PyExtractPointFunction2DAttributes_ToString(const ExtractPointFunction2DAttributes *, const char *, const bool=false);
PyObject *     PyExtractPointFunction2DAttributes_getattr(PyObject *self, char *name);
int            PyExtractPointFunction2DAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyExtractPointFunction2DAttributes_methods[EXTRACTPOINTFUNCTION2DATTRIBUTES_NMETH];

#endif

