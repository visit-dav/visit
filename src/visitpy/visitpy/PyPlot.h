// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PLOT_H
#define PY_PLOT_H
#include <Python.h>
#include <Py2and3Support.h>
#include <Plot.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PLOT_NMETH 46
void VISITPY_API           PyPlot_StartUp(Plot *subj, void *data);
void VISITPY_API           PyPlot_CloseDown();
VISITPY_API PyMethodDef *  PyPlot_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPlot_Check(PyObject *obj);
VISITPY_API Plot *  PyPlot_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPlot_New();
VISITPY_API PyObject *     PyPlot_Wrap(const Plot *attr);
void VISITPY_API           PyPlot_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPlot_SetDefaults(const Plot *atts);
std::string VISITPY_API    PyPlot_GetLogString();
std::string VISITPY_API    PyPlot_ToString(const Plot *, const char *, const bool=false);
VISITPY_API PyObject *     PyPlot_getattr(PyObject *self, char *name);
int VISITPY_API            PyPlot_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPlot_methods[PLOT_NMETH];

#endif

