// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWERCLIENTINFORMATION_H
#define PY_VIEWERCLIENTINFORMATION_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewerClientInformation.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWERCLIENTINFORMATION_NMETH 9
void VISITPY_API           PyViewerClientInformation_StartUp(ViewerClientInformation *subj, void *data);
void VISITPY_API           PyViewerClientInformation_CloseDown();
VISITPY_API PyMethodDef *  PyViewerClientInformation_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewerClientInformation_Check(PyObject *obj);
VISITPY_API ViewerClientInformation *  PyViewerClientInformation_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewerClientInformation_New();
VISITPY_API PyObject *     PyViewerClientInformation_Wrap(const ViewerClientInformation *attr);
void VISITPY_API           PyViewerClientInformation_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewerClientInformation_SetDefaults(const ViewerClientInformation *atts);
std::string VISITPY_API    PyViewerClientInformation_GetLogString();
std::string VISITPY_API    PyViewerClientInformation_ToString(const ViewerClientInformation *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewerClientInformation_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewerClientInformation_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewerClientInformation_methods[VIEWERCLIENTINFORMATION_NMETH];

#endif

