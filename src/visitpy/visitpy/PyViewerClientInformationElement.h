// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWERCLIENTINFORMATIONELEMENT_H
#define PY_VIEWERCLIENTINFORMATIONELEMENT_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewerClientInformationElement.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWERCLIENTINFORMATIONELEMENT_NMETH 12
void VISITPY_API           PyViewerClientInformationElement_StartUp(ViewerClientInformationElement *subj, void *data);
void VISITPY_API           PyViewerClientInformationElement_CloseDown();
VISITPY_API PyMethodDef *  PyViewerClientInformationElement_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewerClientInformationElement_Check(PyObject *obj);
VISITPY_API ViewerClientInformationElement *  PyViewerClientInformationElement_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewerClientInformationElement_New();
VISITPY_API PyObject *     PyViewerClientInformationElement_Wrap(const ViewerClientInformationElement *attr);
void VISITPY_API           PyViewerClientInformationElement_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewerClientInformationElement_SetDefaults(const ViewerClientInformationElement *atts);
std::string VISITPY_API    PyViewerClientInformationElement_GetLogString();
std::string VISITPY_API    PyViewerClientInformationElement_ToString(const ViewerClientInformationElement *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewerClientInformationElement_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewerClientInformationElement_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewerClientInformationElement_methods[VIEWERCLIENTINFORMATIONELEMENT_NMETH];

#endif

