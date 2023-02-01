// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TUBEATTRIBUTES_H
#define PY_TUBEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TubeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TUBEATTRIBUTES_NMETH 16
void           PyTubeAttributes_StartUp(TubeAttributes *subj, void *data);
void           PyTubeAttributes_CloseDown();
PyMethodDef *  PyTubeAttributes_GetMethodTable(int *nMethods);
bool           PyTubeAttributes_Check(PyObject *obj);
TubeAttributes *  PyTubeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTubeAttributes_New();
PyObject *     PyTubeAttributes_Wrap(const TubeAttributes *attr);
void           PyTubeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTubeAttributes_SetDefaults(const TubeAttributes *atts);
std::string    PyTubeAttributes_GetLogString();
std::string    PyTubeAttributes_ToString(const TubeAttributes *, const char *, const bool=false);
PyObject *     PyTubeAttributes_getattr(PyObject *self, char *name);
int            PyTubeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTubeAttributes_methods[TUBEATTRIBUTES_NMETH];

#endif

