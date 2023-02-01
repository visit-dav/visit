// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LAUNCHPROFILE_H
#define PY_LAUNCHPROFILE_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LaunchProfile.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define LAUNCHPROFILE_NMETH 78
void VISITPY_API           PyLaunchProfile_StartUp(LaunchProfile *subj, void *data);
void VISITPY_API           PyLaunchProfile_CloseDown();
VISITPY_API PyMethodDef *  PyLaunchProfile_GetMethodTable(int *nMethods);
bool VISITPY_API           PyLaunchProfile_Check(PyObject *obj);
VISITPY_API LaunchProfile *  PyLaunchProfile_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyLaunchProfile_New();
VISITPY_API PyObject *     PyLaunchProfile_Wrap(const LaunchProfile *attr);
void VISITPY_API           PyLaunchProfile_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyLaunchProfile_SetDefaults(const LaunchProfile *atts);
std::string VISITPY_API    PyLaunchProfile_GetLogString();
std::string VISITPY_API    PyLaunchProfile_ToString(const LaunchProfile *, const char *, const bool=false);
VISITPY_API PyObject *     PyLaunchProfile_getattr(PyObject *self, char *name);
int VISITPY_API            PyLaunchProfile_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyLaunchProfile_methods[LAUNCHPROFILE_NMETH];

#endif

