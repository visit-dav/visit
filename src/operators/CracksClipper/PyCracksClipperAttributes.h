// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CRACKSCLIPPERATTRIBUTES_H
#define PY_CRACKSCLIPPERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CracksClipperAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CRACKSCLIPPERATTRIBUTES_NMETH 18
void           PyCracksClipperAttributes_StartUp(CracksClipperAttributes *subj, void *data);
void           PyCracksClipperAttributes_CloseDown();
PyMethodDef *  PyCracksClipperAttributes_GetMethodTable(int *nMethods);
bool           PyCracksClipperAttributes_Check(PyObject *obj);
CracksClipperAttributes *  PyCracksClipperAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCracksClipperAttributes_New();
PyObject *     PyCracksClipperAttributes_Wrap(const CracksClipperAttributes *attr);
void           PyCracksClipperAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCracksClipperAttributes_SetDefaults(const CracksClipperAttributes *atts);
std::string    PyCracksClipperAttributes_GetLogString();
std::string    PyCracksClipperAttributes_ToString(const CracksClipperAttributes *, const char *, const bool=false);
PyObject *     PyCracksClipperAttributes_getattr(PyObject *self, char *name);
int            PyCracksClipperAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCracksClipperAttributes_methods[CRACKSCLIPPERATTRIBUTES_NMETH];

#endif

