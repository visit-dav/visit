// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VOLUMEATTRIBUTES_H
#define PY_VOLUMEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <VolumeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define VOLUMEATTRIBUTES_NMETH 86
void           PyVolumeAttributes_StartUp(VolumeAttributes *subj, void *data);
void           PyVolumeAttributes_CloseDown();
PyMethodDef *  PyVolumeAttributes_GetMethodTable(int *nMethods);
bool           PyVolumeAttributes_Check(PyObject *obj);
VolumeAttributes *  PyVolumeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyVolumeAttributes_New();
PyObject *     PyVolumeAttributes_Wrap(const VolumeAttributes *attr);
void           PyVolumeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyVolumeAttributes_SetDefaults(const VolumeAttributes *atts);
std::string    PyVolumeAttributes_GetLogString();
std::string    PyVolumeAttributes_ToString(const VolumeAttributes *, const char *, const bool=false);
PyObject *     PyVolumeAttributes_getattr(PyObject *self, char *name);
int            PyVolumeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyVolumeAttributes_methods[VOLUMEATTRIBUTES_NMETH];

#endif

