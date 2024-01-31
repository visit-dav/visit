// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWERRPC_H
#define PY_VIEWERRPC_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewerRPC.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWERRPC_NMETH 68
void VISITPY_API           PyViewerRPC_StartUp(ViewerRPC *subj, void *data);
void VISITPY_API           PyViewerRPC_CloseDown();
VISITPY_API PyMethodDef *  PyViewerRPC_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewerRPC_Check(PyObject *obj);
VISITPY_API ViewerRPC *  PyViewerRPC_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewerRPC_New();
VISITPY_API PyObject *     PyViewerRPC_Wrap(const ViewerRPC *attr);
void VISITPY_API           PyViewerRPC_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewerRPC_SetDefaults(const ViewerRPC *atts);
std::string VISITPY_API    PyViewerRPC_GetLogString();
std::string VISITPY_API    PyViewerRPC_ToString(const ViewerRPC *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewerRPC_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewerRPC_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewerRPC_methods[VIEWERRPC_NMETH];

#endif

