// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COLORCONTROLPOINTLIST_H
#define PY_COLORCONTROLPOINTLIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ColorControlPointList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define COLORCONTROLPOINTLIST_NMETH 16
void VISITPY_API           PyColorControlPointList_StartUp(ColorControlPointList *subj, void *data);
void VISITPY_API           PyColorControlPointList_CloseDown();
VISITPY_API PyMethodDef *  PyColorControlPointList_GetMethodTable(int *nMethods);
bool VISITPY_API           PyColorControlPointList_Check(PyObject *obj);
VISITPY_API ColorControlPointList *  PyColorControlPointList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyColorControlPointList_New();
VISITPY_API PyObject *     PyColorControlPointList_Wrap(const ColorControlPointList *attr);
void VISITPY_API           PyColorControlPointList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyColorControlPointList_SetDefaults(const ColorControlPointList *atts);
std::string VISITPY_API    PyColorControlPointList_GetLogString();
std::string VISITPY_API    PyColorControlPointList_ToString(const ColorControlPointList *, const char *, const bool=false);
VISITPY_API PyObject *     PyColorControlPointList_getattr(PyObject *self, char *name);
int VISITPY_API            PyColorControlPointList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyColorControlPointList_methods[COLORCONTROLPOINTLIST_NMETH];

#endif

