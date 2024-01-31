// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MODELFITATTS_H
#define PY_MODELFITATTS_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ModelFitAtts.h>

//
// Functions exposed to the VisIt module.
//
#define MODELFITATTS_NMETH 24
void           PyModelFitAtts_StartUp(ModelFitAtts *subj, void *data);
void           PyModelFitAtts_CloseDown();
PyMethodDef *  PyModelFitAtts_GetMethodTable(int *nMethods);
bool           PyModelFitAtts_Check(PyObject *obj);
ModelFitAtts *  PyModelFitAtts_FromPyObject(PyObject *obj);
PyObject *     PyModelFitAtts_New();
PyObject *     PyModelFitAtts_Wrap(const ModelFitAtts *attr);
void           PyModelFitAtts_SetParent(PyObject *obj, PyObject *parent);
void           PyModelFitAtts_SetDefaults(const ModelFitAtts *atts);
std::string    PyModelFitAtts_GetLogString();
std::string    PyModelFitAtts_ToString(const ModelFitAtts *, const char *, const bool=false);
PyObject *     PyModelFitAtts_getattr(PyObject *self, char *name);
int            PyModelFitAtts_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyModelFitAtts_methods[MODELFITATTS_NMETH];

#endif

