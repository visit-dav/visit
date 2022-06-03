// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTDEFAULTPLOTMETADATA_H
#define PY_AVTDEFAULTPLOTMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtDefaultPlotMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTDEFAULTPLOTMETADATA_NMETH 8
void VISITPY_API           PyavtDefaultPlotMetaData_StartUp(avtDefaultPlotMetaData *subj, void *data);
void VISITPY_API           PyavtDefaultPlotMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtDefaultPlotMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtDefaultPlotMetaData_Check(PyObject *obj);
VISITPY_API avtDefaultPlotMetaData *  PyavtDefaultPlotMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtDefaultPlotMetaData_New();
VISITPY_API PyObject *     PyavtDefaultPlotMetaData_Wrap(const avtDefaultPlotMetaData *attr);
void VISITPY_API           PyavtDefaultPlotMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtDefaultPlotMetaData_SetDefaults(const avtDefaultPlotMetaData *atts);
std::string VISITPY_API    PyavtDefaultPlotMetaData_GetLogString();
std::string VISITPY_API    PyavtDefaultPlotMetaData_ToString(const avtDefaultPlotMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtDefaultPlotMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtDefaultPlotMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtDefaultPlotMetaData_methods[AVTDEFAULTPLOTMETADATA_NMETH];

#endif

