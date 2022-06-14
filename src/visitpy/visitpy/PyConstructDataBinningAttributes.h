// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONSTRUCTDATABINNINGATTRIBUTES_H
#define PY_CONSTRUCTDATABINNINGATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ConstructDataBinningAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define CONSTRUCTDATABINNINGATTRIBUTES_NMETH 30
void VISITPY_API           PyConstructDataBinningAttributes_StartUp(ConstructDataBinningAttributes *subj, void *data);
void VISITPY_API           PyConstructDataBinningAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyConstructDataBinningAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyConstructDataBinningAttributes_Check(PyObject *obj);
VISITPY_API ConstructDataBinningAttributes *  PyConstructDataBinningAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyConstructDataBinningAttributes_New();
VISITPY_API PyObject *     PyConstructDataBinningAttributes_Wrap(const ConstructDataBinningAttributes *attr);
void VISITPY_API           PyConstructDataBinningAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyConstructDataBinningAttributes_SetDefaults(const ConstructDataBinningAttributes *atts);
std::string VISITPY_API    PyConstructDataBinningAttributes_GetLogString();
std::string VISITPY_API    PyConstructDataBinningAttributes_ToString(const ConstructDataBinningAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyConstructDataBinningAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyConstructDataBinningAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyConstructDataBinningAttributes_methods[CONSTRUCTDATABINNINGATTRIBUTES_NMETH];

#endif

