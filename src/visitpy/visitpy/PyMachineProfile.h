// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MACHINEPROFILE_H
#define PY_MACHINEPROFILE_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MachineProfile.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define MACHINEPROFILE_NMETH 49
void VISITPY_API           PyMachineProfile_StartUp(MachineProfile *subj, void *data);
void VISITPY_API           PyMachineProfile_CloseDown();
VISITPY_API PyMethodDef *  PyMachineProfile_GetMethodTable(int *nMethods);
bool VISITPY_API           PyMachineProfile_Check(PyObject *obj);
VISITPY_API MachineProfile *  PyMachineProfile_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyMachineProfile_New();
VISITPY_API PyObject *     PyMachineProfile_Wrap(const MachineProfile *attr);
void VISITPY_API           PyMachineProfile_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyMachineProfile_SetDefaults(const MachineProfile *atts);
std::string VISITPY_API    PyMachineProfile_GetLogString();
std::string VISITPY_API    PyMachineProfile_ToString(const MachineProfile *, const char *, const bool=false);
VISITPY_API PyObject *     PyMachineProfile_getattr(PyObject *self, char *name);
int VISITPY_API            PyMachineProfile_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyMachineProfile_methods[MACHINEPROFILE_NMETH];

#endif

