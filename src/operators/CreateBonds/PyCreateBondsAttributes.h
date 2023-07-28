// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CREATEBONDSATTRIBUTES_H
#define PY_CREATEBONDSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CreateBondsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CREATEBONDSATTRIBUTES_NMETH 30
void           PyCreateBondsAttributes_StartUp(CreateBondsAttributes *subj, void *data);
void           PyCreateBondsAttributes_CloseDown();
PyMethodDef *  PyCreateBondsAttributes_GetMethodTable(int *nMethods);
bool           PyCreateBondsAttributes_Check(PyObject *obj);
CreateBondsAttributes *  PyCreateBondsAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCreateBondsAttributes_New();
PyObject *     PyCreateBondsAttributes_Wrap(const CreateBondsAttributes *attr);
void           PyCreateBondsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCreateBondsAttributes_SetDefaults(const CreateBondsAttributes *atts);
std::string    PyCreateBondsAttributes_GetLogString();
std::string    PyCreateBondsAttributes_ToString(const CreateBondsAttributes *, const char *, const bool=false);
PyObject *     PyCreateBondsAttributes_getattr(PyObject *self, char *name);
int            PyCreateBondsAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCreateBondsAttributes_methods[CREATEBONDSATTRIBUTES_NMETH];

#endif

