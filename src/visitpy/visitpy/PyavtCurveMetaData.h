// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTCURVEMETADATA_H
#define PY_AVTCURVEMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtCurveMetaData.h>
#include <PyavtVarMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
void VISITPY_API           PyavtCurveMetaData_StartUp(avtCurveMetaData *subj, void *data);
void VISITPY_API           PyavtCurveMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtCurveMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtCurveMetaData_Check(PyObject *obj);
VISITPY_API avtCurveMetaData *  PyavtCurveMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtCurveMetaData_New();
VISITPY_API PyObject *     PyavtCurveMetaData_Wrap(const avtCurveMetaData *attr);
void VISITPY_API           PyavtCurveMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtCurveMetaData_SetDefaults(const avtCurveMetaData *atts);
std::string VISITPY_API    PyavtCurveMetaData_GetLogString();
std::string VISITPY_API    PyavtCurveMetaData_ToString(const avtCurveMetaData *, const char *, const bool=false);

#endif

