// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SPREADSHEETATTRIBUTES_H
#define PY_SPREADSHEETATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SpreadsheetAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SPREADSHEETATTRIBUTES_NMETH 30
void           PySpreadsheetAttributes_StartUp(SpreadsheetAttributes *subj, void *data);
void           PySpreadsheetAttributes_CloseDown();
PyMethodDef *  PySpreadsheetAttributes_GetMethodTable(int *nMethods);
bool           PySpreadsheetAttributes_Check(PyObject *obj);
SpreadsheetAttributes *  PySpreadsheetAttributes_FromPyObject(PyObject *obj);
PyObject *     PySpreadsheetAttributes_New();
PyObject *     PySpreadsheetAttributes_Wrap(const SpreadsheetAttributes *attr);
void           PySpreadsheetAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySpreadsheetAttributes_SetDefaults(const SpreadsheetAttributes *atts);
std::string    PySpreadsheetAttributes_GetLogString();
std::string    PySpreadsheetAttributes_ToString(const SpreadsheetAttributes *, const char *, const bool=false);
PyObject *     PySpreadsheetAttributes_getattr(PyObject *self, char *name);
int            PySpreadsheetAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySpreadsheetAttributes_methods[SPREADSHEETATTRIBUTES_NMETH];

#endif

