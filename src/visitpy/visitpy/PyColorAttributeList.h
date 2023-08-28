// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COLORATTRIBUTELIST_H
#define PY_COLORATTRIBUTELIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ColorAttributeList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define COLORATTRIBUTELIST_NMETH 7
void VISITPY_API           PyColorAttributeList_StartUp(ColorAttributeList *subj, void *data);
void VISITPY_API           PyColorAttributeList_CloseDown();
VISITPY_API PyMethodDef *  PyColorAttributeList_GetMethodTable(int *nMethods);
bool VISITPY_API           PyColorAttributeList_Check(PyObject *obj);
VISITPY_API ColorAttributeList *  PyColorAttributeList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyColorAttributeList_New();
VISITPY_API PyObject *     PyColorAttributeList_Wrap(const ColorAttributeList *attr);
void VISITPY_API           PyColorAttributeList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyColorAttributeList_SetDefaults(const ColorAttributeList *atts);
std::string VISITPY_API    PyColorAttributeList_GetLogString();
std::string VISITPY_API    PyColorAttributeList_ToString(const ColorAttributeList *, const char *, const bool=false);
VISITPY_API PyObject *     PyColorAttributeList_getattr(PyObject *self, char *name);
int VISITPY_API            PyColorAttributeList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyColorAttributeList_methods[COLORATTRIBUTELIST_NMETH];

#endif

