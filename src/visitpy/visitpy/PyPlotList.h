// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PLOTLIST_H
#define PY_PLOTLIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PlotList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PLOTLIST_NMETH 7
void VISITPY_API           PyPlotList_StartUp(PlotList *subj, void *data);
void VISITPY_API           PyPlotList_CloseDown();
VISITPY_API PyMethodDef *  PyPlotList_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPlotList_Check(PyObject *obj);
VISITPY_API PlotList *  PyPlotList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPlotList_New();
VISITPY_API PyObject *     PyPlotList_Wrap(const PlotList *attr);
void VISITPY_API           PyPlotList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPlotList_SetDefaults(const PlotList *atts);
std::string VISITPY_API    PyPlotList_GetLogString();
std::string VISITPY_API    PyPlotList_ToString(const PlotList *, const char *, const bool=false);
VISITPY_API PyObject *     PyPlotList_getattr(PyObject *self, char *name);
int VISITPY_API            PyPlotList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPlotList_methods[PLOTLIST_NMETH];

#endif

