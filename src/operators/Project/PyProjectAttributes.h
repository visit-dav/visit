// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PROJECTATTRIBUTES_H
#define PY_PROJECTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ProjectAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PROJECTATTRIBUTES_NMETH 6
void           PyProjectAttributes_StartUp(ProjectAttributes *subj, void *data);
void           PyProjectAttributes_CloseDown();
PyMethodDef *  PyProjectAttributes_GetMethodTable(int *nMethods);
bool           PyProjectAttributes_Check(PyObject *obj);
ProjectAttributes *  PyProjectAttributes_FromPyObject(PyObject *obj);
PyObject *     PyProjectAttributes_New();
PyObject *     PyProjectAttributes_Wrap(const ProjectAttributes *attr);
void           PyProjectAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyProjectAttributes_SetDefaults(const ProjectAttributes *atts);
std::string    PyProjectAttributes_GetLogString();
std::string    PyProjectAttributes_ToString(const ProjectAttributes *, const char *, const bool=false);
PyObject *     PyProjectAttributes_getattr(PyObject *self, char *name);
int            PyProjectAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyProjectAttributes_methods[PROJECTATTRIBUTES_NMETH];

#endif

