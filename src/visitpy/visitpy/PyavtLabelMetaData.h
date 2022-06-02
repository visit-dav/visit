// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTLABELMETADATA_H
#define PY_AVTLABELMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtLabelMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTLABELMETADATA_NMETH (AVTVARMETADATA_NMETH+2)
void VISITPY_API           PyavtLabelMetaData_StartUp(avtLabelMetaData *subj, void *data);
void VISITPY_API           PyavtLabelMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtLabelMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtLabelMetaData_Check(PyObject *obj);
VISITPY_API avtLabelMetaData *  PyavtLabelMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtLabelMetaData_New();
VISITPY_API PyObject *     PyavtLabelMetaData_Wrap(const avtLabelMetaData *attr);
void VISITPY_API           PyavtLabelMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtLabelMetaData_SetDefaults(const avtLabelMetaData *atts);
std::string VISITPY_API    PyavtLabelMetaData_GetLogString();
std::string VISITPY_API    PyavtLabelMetaData_ToString(const avtLabelMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtLabelMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtLabelMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtLabelMetaData_methods[AVTLABELMETADATA_NMETH];

#endif

