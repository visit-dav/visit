// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONTOURATTRIBUTES_H
#define PY_CONTOURATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ContourAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PyContourAttributes_StartUp(ContourAttributes *subj, void *data);
void           PyContourAttributes_CloseDown();
PyMethodDef *  PyContourAttributes_GetMethodTable(int *nMethods);
bool           PyContourAttributes_Check(PyObject *obj);
ContourAttributes *  PyContourAttributes_FromPyObject(PyObject *obj);
PyObject *     PyContourAttributes_New();
PyObject *     PyContourAttributes_Wrap(const ContourAttributes *attr);
void           PyContourAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyContourAttributes_SetDefaults(const ContourAttributes *atts);
std::string    PyContourAttributes_GetLogString();
std::string    PyContourAttributes_ToString(const ContourAttributes *, const char *, const bool=false);

#endif

