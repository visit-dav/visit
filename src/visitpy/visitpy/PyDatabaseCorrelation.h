// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DATABASECORRELATION_H
#define PY_DATABASECORRELATION_H
#include <Python.h>
#include <DatabaseCorrelation.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
void VISITPY_API           PyDatabaseCorrelation_StartUp(DatabaseCorrelation *subj, void *data);
void VISITPY_API           PyDatabaseCorrelation_CloseDown();
VISITPY_API PyMethodDef *   PyDatabaseCorrelation_GetMethodTable(int *nMethods);
bool VISITPY_API           PyDatabaseCorrelation_Check(PyObject *obj);
VISITPY_API DatabaseCorrelation *  PyDatabaseCorrelation_FromPyObject(PyObject *obj);
VISITPY_API PyObject *      PyDatabaseCorrelation_New();
VISITPY_API PyObject *      PyDatabaseCorrelation_Wrap(const DatabaseCorrelation *attr);
void VISITPY_API           PyDatabaseCorrelation_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyDatabaseCorrelation_SetDefaults(const DatabaseCorrelation *atts);
std::string VISITPY_API    PyDatabaseCorrelation_GetLogString();
std::string VISITPY_API    PyDatabaseCorrelation_ToString(const DatabaseCorrelation *, const char *);

#endif

