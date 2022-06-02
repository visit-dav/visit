// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_REMAPATTRIBUTES_H
#define PY_REMAPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <RemapAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define REMAPATTRIBUTES_NMETH 26
void           PyRemapAttributes_StartUp(RemapAttributes *subj, void *data);
void           PyRemapAttributes_CloseDown();
PyMethodDef *  PyRemapAttributes_GetMethodTable(int *nMethods);
bool           PyRemapAttributes_Check(PyObject *obj);
RemapAttributes *  PyRemapAttributes_FromPyObject(PyObject *obj);
PyObject *     PyRemapAttributes_New();
PyObject *     PyRemapAttributes_Wrap(const RemapAttributes *attr);
void           PyRemapAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyRemapAttributes_SetDefaults(const RemapAttributes *atts);
std::string    PyRemapAttributes_GetLogString();
std::string    PyRemapAttributes_ToString(const RemapAttributes *, const char *, const bool=false);
PyObject *     PyRemapAttributes_getattr(PyObject *self, char *name);
int            PyRemapAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyRemapAttributes_methods[REMAPATTRIBUTES_NMETH];

#endif

