// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_RENDERINGATTRIBUTES_H
#define PY_RENDERINGATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <RenderingAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define RENDERINGATTRIBUTES_NMETH 72
void VISITPY_API           PyRenderingAttributes_StartUp(RenderingAttributes *subj, void *data);
void VISITPY_API           PyRenderingAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyRenderingAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyRenderingAttributes_Check(PyObject *obj);
VISITPY_API RenderingAttributes *  PyRenderingAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyRenderingAttributes_New();
VISITPY_API PyObject *     PyRenderingAttributes_Wrap(const RenderingAttributes *attr);
void VISITPY_API           PyRenderingAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyRenderingAttributes_SetDefaults(const RenderingAttributes *atts);
std::string VISITPY_API    PyRenderingAttributes_GetLogString();
std::string VISITPY_API    PyRenderingAttributes_ToString(const RenderingAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyRenderingAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyRenderingAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyRenderingAttributes_methods[RENDERINGATTRIBUTES_NMETH];

#endif

