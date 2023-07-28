// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SUBDIVIDEQUADSATTRIBUTES_H
#define PY_SUBDIVIDEQUADSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SubdivideQuadsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SUBDIVIDEQUADSATTRIBUTES_NMETH 12
void           PySubdivideQuadsAttributes_StartUp(SubdivideQuadsAttributes *subj, void *data);
void           PySubdivideQuadsAttributes_CloseDown();
PyMethodDef *  PySubdivideQuadsAttributes_GetMethodTable(int *nMethods);
bool           PySubdivideQuadsAttributes_Check(PyObject *obj);
SubdivideQuadsAttributes *  PySubdivideQuadsAttributes_FromPyObject(PyObject *obj);
PyObject *     PySubdivideQuadsAttributes_New();
PyObject *     PySubdivideQuadsAttributes_Wrap(const SubdivideQuadsAttributes *attr);
void           PySubdivideQuadsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySubdivideQuadsAttributes_SetDefaults(const SubdivideQuadsAttributes *atts);
std::string    PySubdivideQuadsAttributes_GetLogString();
std::string    PySubdivideQuadsAttributes_ToString(const SubdivideQuadsAttributes *, const char *, const bool=false);
PyObject *     PySubdivideQuadsAttributes_getattr(PyObject *self, char *name);
int            PySubdivideQuadsAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySubdivideQuadsAttributes_methods[SUBDIVIDEQUADSATTRIBUTES_NMETH];

#endif

