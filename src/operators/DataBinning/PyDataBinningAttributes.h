// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DATABINNINGATTRIBUTES_H
#define PY_DATABINNINGATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DataBinningAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DATABINNINGATTRIBUTES_NMETH 52
void           PyDataBinningAttributes_StartUp(DataBinningAttributes *subj, void *data);
void           PyDataBinningAttributes_CloseDown();
PyMethodDef *  PyDataBinningAttributes_GetMethodTable(int *nMethods);
bool           PyDataBinningAttributes_Check(PyObject *obj);
DataBinningAttributes *  PyDataBinningAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDataBinningAttributes_New();
PyObject *     PyDataBinningAttributes_Wrap(const DataBinningAttributes *attr);
void           PyDataBinningAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDataBinningAttributes_SetDefaults(const DataBinningAttributes *atts);
std::string    PyDataBinningAttributes_GetLogString();
std::string    PyDataBinningAttributes_ToString(const DataBinningAttributes *, const char *, const bool=false);
PyObject *     PyDataBinningAttributes_getattr(PyObject *self, char *name);
int            PyDataBinningAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDataBinningAttributes_methods[DATABINNINGATTRIBUTES_NMETH];

#endif

