// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COLORCONTROLPOINT_H
#define PY_COLORCONTROLPOINT_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ColorControlPoint.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define COLORCONTROLPOINT_NMETH 6
void VISITPY_API           PyColorControlPoint_StartUp(ColorControlPoint *subj, void *data);
void VISITPY_API           PyColorControlPoint_CloseDown();
VISITPY_API PyMethodDef *  PyColorControlPoint_GetMethodTable(int *nMethods);
bool VISITPY_API           PyColorControlPoint_Check(PyObject *obj);
VISITPY_API ColorControlPoint *  PyColorControlPoint_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyColorControlPoint_New();
VISITPY_API PyObject *     PyColorControlPoint_Wrap(const ColorControlPoint *attr);
void VISITPY_API           PyColorControlPoint_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyColorControlPoint_SetDefaults(const ColorControlPoint *atts);
std::string VISITPY_API    PyColorControlPoint_GetLogString();
std::string VISITPY_API    PyColorControlPoint_ToString(const ColorControlPoint *, const char *, const bool=false);
VISITPY_API PyObject *     PyColorControlPoint_getattr(PyObject *self, char *name);
int VISITPY_API            PyColorControlPoint_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyColorControlPoint_methods[COLORCONTROLPOINT_NMETH];

#endif

