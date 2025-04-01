// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CLIPATTRIBUTES_H
#define PY_CLIPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ClipAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PyClipAttributes_StartUp(ClipAttributes *subj, void *data);
void           PyClipAttributes_CloseDown();
PyMethodDef *  PyClipAttributes_GetMethodTable(int *nMethods);
bool           PyClipAttributes_Check(PyObject *obj);
ClipAttributes *  PyClipAttributes_FromPyObject(PyObject *obj);
PyObject *     PyClipAttributes_New();
PyObject *     PyClipAttributes_Wrap(const ClipAttributes *attr);
void           PyClipAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyClipAttributes_SetDefaults(const ClipAttributes *atts);
std::string    PyClipAttributes_GetLogString();
std::string    PyClipAttributes_ToString(const ClipAttributes *, const char *, const bool=false);

#endif

