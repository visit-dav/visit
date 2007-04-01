#ifndef PY_DATABASECORRELATION_H
#define PY_DATABASECORRELATION_H
#include <Python.h>
#include <DatabaseCorrelation.h>

//
// Functions exposed to the VisIt module.
//
bool            PyDatabaseCorrelation_Check(PyObject *obj);
DatabaseCorrelation *PyDatabaseCorrelation_FromPyObject(PyObject *obj);
PyObject       *PyDatabaseCorrelation_NewPyObject(const DatabaseCorrelation *attr);
PyObject       *PyDatabaseCorrelation_WrapPyObject(const DatabaseCorrelation *attr);

PyObject       *PyDatabaseCorrelation_StringRepresentation(const DatabaseCorrelation *atts);

#endif

