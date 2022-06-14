// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_KEYFRAMEATTRIBUTES_H
#define PY_KEYFRAMEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <KeyframeAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define KEYFRAMEATTRIBUTES_NMETH 8
void VISITPY_API           PyKeyframeAttributes_StartUp(KeyframeAttributes *subj, void *data);
void VISITPY_API           PyKeyframeAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyKeyframeAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyKeyframeAttributes_Check(PyObject *obj);
VISITPY_API KeyframeAttributes *  PyKeyframeAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyKeyframeAttributes_New();
VISITPY_API PyObject *     PyKeyframeAttributes_Wrap(const KeyframeAttributes *attr);
void VISITPY_API           PyKeyframeAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyKeyframeAttributes_SetDefaults(const KeyframeAttributes *atts);
std::string VISITPY_API    PyKeyframeAttributes_GetLogString();
std::string VISITPY_API    PyKeyframeAttributes_ToString(const KeyframeAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyKeyframeAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyKeyframeAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyKeyframeAttributes_methods[KEYFRAMEATTRIBUTES_NMETH];

#endif

