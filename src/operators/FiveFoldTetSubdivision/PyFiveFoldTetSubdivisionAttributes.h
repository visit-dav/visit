// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FIVEFOLDTETSUBDIVISIONATTRIBUTES_H
#define PY_FIVEFOLDTETSUBDIVISIONATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FiveFoldTetSubdivisionAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define FIVEFOLDTETSUBDIVISIONATTRIBUTES_NMETH 18
void           PyFiveFoldTetSubdivisionAttributes_StartUp(FiveFoldTetSubdivisionAttributes *subj, void *data);
void           PyFiveFoldTetSubdivisionAttributes_CloseDown();
PyMethodDef *  PyFiveFoldTetSubdivisionAttributes_GetMethodTable(int *nMethods);
bool           PyFiveFoldTetSubdivisionAttributes_Check(PyObject *obj);
FiveFoldTetSubdivisionAttributes *  PyFiveFoldTetSubdivisionAttributes_FromPyObject(PyObject *obj);
PyObject *     PyFiveFoldTetSubdivisionAttributes_New();
PyObject *     PyFiveFoldTetSubdivisionAttributes_Wrap(const FiveFoldTetSubdivisionAttributes *attr);
void           PyFiveFoldTetSubdivisionAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyFiveFoldTetSubdivisionAttributes_SetDefaults(const FiveFoldTetSubdivisionAttributes *atts);
std::string    PyFiveFoldTetSubdivisionAttributes_GetLogString();
std::string    PyFiveFoldTetSubdivisionAttributes_ToString(const FiveFoldTetSubdivisionAttributes *, const char *, const bool=false);
PyObject *     PyFiveFoldTetSubdivisionAttributes_getattr(PyObject *self, char *name);
int            PyFiveFoldTetSubdivisionAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyFiveFoldTetSubdivisionAttributes_methods[FIVEFOLDTETSUBDIVISIONATTRIBUTES_NMETH];

#endif

