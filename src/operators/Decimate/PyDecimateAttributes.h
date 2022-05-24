// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DECIMATEATTRIBUTES_H
#define PY_DECIMATEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <DecimateAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define DECIMATEATTRIBUTES_NMETH 4
void           PyDecimateAttributes_StartUp(DecimateAttributes *subj, void *data);
void           PyDecimateAttributes_CloseDown();
PyMethodDef *  PyDecimateAttributes_GetMethodTable(int *nMethods);
bool           PyDecimateAttributes_Check(PyObject *obj);
DecimateAttributes *  PyDecimateAttributes_FromPyObject(PyObject *obj);
PyObject *     PyDecimateAttributes_New();
PyObject *     PyDecimateAttributes_Wrap(const DecimateAttributes *attr);
void           PyDecimateAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyDecimateAttributes_SetDefaults(const DecimateAttributes *atts);
std::string    PyDecimateAttributes_GetLogString();
std::string    PyDecimateAttributes_ToString(const DecimateAttributes *, const char *, const bool=false);
PyObject *     PyDecimateAttributes_getattr(PyObject *self, char *name);
int            PyDecimateAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyDecimateAttributes_methods[DECIMATEATTRIBUTES_NMETH];

#endif

