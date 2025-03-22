// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TRIANGULATEREGULARPOINTSATTRIBUTES_H
#define PY_TRIANGULATEREGULARPOINTSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TriangulateRegularPointsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TRIANGULATEREGULARPOINTSATTRIBUTES_NMETH 11
void           PyTriangulateRegularPointsAttributes_StartUp(TriangulateRegularPointsAttributes *subj, void *data);
void           PyTriangulateRegularPointsAttributes_CloseDown();
PyMethodDef *  PyTriangulateRegularPointsAttributes_GetMethodTable(int *nMethods);
bool           PyTriangulateRegularPointsAttributes_Check(PyObject *obj);
TriangulateRegularPointsAttributes *  PyTriangulateRegularPointsAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTriangulateRegularPointsAttributes_New();
PyObject *     PyTriangulateRegularPointsAttributes_Wrap(const TriangulateRegularPointsAttributes *attr);
void           PyTriangulateRegularPointsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTriangulateRegularPointsAttributes_SetDefaults(const TriangulateRegularPointsAttributes *atts);
std::string    PyTriangulateRegularPointsAttributes_GetLogString();
std::string    PyTriangulateRegularPointsAttributes_ToString(const TriangulateRegularPointsAttributes *, const char *, const bool=false);
PyObject *     PyTriangulateRegularPointsAttributes_getattro(PyObject *self, PyObject *attr_name);
int            PyTriangulateRegularPointsAttributes_setattro(PyObject *self, PyObject *attr_name, PyObject *args);
extern PyMethodDef PyTriangulateRegularPointsAttributes_methods[TRIANGULATEREGULARPOINTSATTRIBUTES_NMETH];

#endif

