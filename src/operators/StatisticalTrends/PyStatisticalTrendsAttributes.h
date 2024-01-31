// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_STATISTICALTRENDSATTRIBUTES_H
#define PY_STATISTICALTRENDSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <StatisticalTrendsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define STATISTICALTRENDSATTRIBUTES_NMETH 18
void           PyStatisticalTrendsAttributes_StartUp(StatisticalTrendsAttributes *subj, void *data);
void           PyStatisticalTrendsAttributes_CloseDown();
PyMethodDef *  PyStatisticalTrendsAttributes_GetMethodTable(int *nMethods);
bool           PyStatisticalTrendsAttributes_Check(PyObject *obj);
StatisticalTrendsAttributes *  PyStatisticalTrendsAttributes_FromPyObject(PyObject *obj);
PyObject *     PyStatisticalTrendsAttributes_New();
PyObject *     PyStatisticalTrendsAttributes_Wrap(const StatisticalTrendsAttributes *attr);
void           PyStatisticalTrendsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyStatisticalTrendsAttributes_SetDefaults(const StatisticalTrendsAttributes *atts);
std::string    PyStatisticalTrendsAttributes_GetLogString();
std::string    PyStatisticalTrendsAttributes_ToString(const StatisticalTrendsAttributes *, const char *, const bool=false);
PyObject *     PyStatisticalTrendsAttributes_getattr(PyObject *self, char *name);
int            PyStatisticalTrendsAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyStatisticalTrendsAttributes_methods[STATISTICALTRENDSATTRIBUTES_NMETH];

#endif

