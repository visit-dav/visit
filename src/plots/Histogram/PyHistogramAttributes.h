// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_HISTOGRAMATTRIBUTES_H
#define PY_HISTOGRAMATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <HistogramAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define HISTOGRAMATTRIBUTES_NMETH 40
void           PyHistogramAttributes_StartUp(HistogramAttributes *subj, void *data);
void           PyHistogramAttributes_CloseDown();
PyMethodDef *  PyHistogramAttributes_GetMethodTable(int *nMethods);
bool           PyHistogramAttributes_Check(PyObject *obj);
HistogramAttributes *  PyHistogramAttributes_FromPyObject(PyObject *obj);
PyObject *     PyHistogramAttributes_New();
PyObject *     PyHistogramAttributes_Wrap(const HistogramAttributes *attr);
void           PyHistogramAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyHistogramAttributes_SetDefaults(const HistogramAttributes *atts);
std::string    PyHistogramAttributes_GetLogString();
std::string    PyHistogramAttributes_ToString(const HistogramAttributes *, const char *, const bool=false);
PyObject *     PyHistogramAttributes_getattr(PyObject *self, char *name);
int            PyHistogramAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyHistogramAttributes_methods[HISTOGRAMATTRIBUTES_NMETH];

#endif

