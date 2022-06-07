// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FILEOPENOPTIONS_H
#define PY_FILEOPENOPTIONS_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FileOpenOptions.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define FILEOPENOPTIONS_NMETH 15
void VISITPY_API           PyFileOpenOptions_StartUp(FileOpenOptions *subj, void *data);
void VISITPY_API           PyFileOpenOptions_CloseDown();
VISITPY_API PyMethodDef *  PyFileOpenOptions_GetMethodTable(int *nMethods);
bool VISITPY_API           PyFileOpenOptions_Check(PyObject *obj);
VISITPY_API FileOpenOptions *  PyFileOpenOptions_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyFileOpenOptions_New();
VISITPY_API PyObject *     PyFileOpenOptions_Wrap(const FileOpenOptions *attr);
void VISITPY_API           PyFileOpenOptions_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyFileOpenOptions_SetDefaults(const FileOpenOptions *atts);
std::string VISITPY_API    PyFileOpenOptions_GetLogString();
std::string VISITPY_API    PyFileOpenOptions_ToString(const FileOpenOptions *, const char *, const bool=false);
VISITPY_API PyObject *     PyFileOpenOptions_getattr(PyObject *self, char *name);
int VISITPY_API            PyFileOpenOptions_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyFileOpenOptions_methods[FILEOPENOPTIONS_NMETH];

#endif

