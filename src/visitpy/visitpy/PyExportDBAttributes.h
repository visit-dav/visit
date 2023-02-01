// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXPORTDBATTRIBUTES_H
#define PY_EXPORTDBATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExportDBAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define EXPORTDBATTRIBUTES_NMETH 22
void VISITPY_API           PyExportDBAttributes_StartUp(ExportDBAttributes *subj, void *data);
void VISITPY_API           PyExportDBAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyExportDBAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyExportDBAttributes_Check(PyObject *obj);
VISITPY_API ExportDBAttributes *  PyExportDBAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyExportDBAttributes_New();
VISITPY_API PyObject *     PyExportDBAttributes_Wrap(const ExportDBAttributes *attr);
void VISITPY_API           PyExportDBAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyExportDBAttributes_SetDefaults(const ExportDBAttributes *atts);
std::string VISITPY_API    PyExportDBAttributes_GetLogString();
std::string VISITPY_API    PyExportDBAttributes_ToString(const ExportDBAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyExportDBAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyExportDBAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyExportDBAttributes_methods[EXPORTDBATTRIBUTES_NMETH];

#endif

