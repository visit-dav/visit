// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWERCLIENTATTRIBUTES_H
#define PY_VIEWERCLIENTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewerClientAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWERCLIENTATTRIBUTES_NMETH 20
void VISITPY_API           PyViewerClientAttributes_StartUp(ViewerClientAttributes *subj, void *data);
void VISITPY_API           PyViewerClientAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyViewerClientAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewerClientAttributes_Check(PyObject *obj);
VISITPY_API ViewerClientAttributes *  PyViewerClientAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewerClientAttributes_New();
VISITPY_API PyObject *     PyViewerClientAttributes_Wrap(const ViewerClientAttributes *attr);
void VISITPY_API           PyViewerClientAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewerClientAttributes_SetDefaults(const ViewerClientAttributes *atts);
std::string VISITPY_API    PyViewerClientAttributes_GetLogString();
std::string VISITPY_API    PyViewerClientAttributes_ToString(const ViewerClientAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewerClientAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewerClientAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewerClientAttributes_methods[VIEWERCLIENTATTRIBUTES_NMETH];

#endif

