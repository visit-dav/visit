// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FFTATTRIBUTES_H
#define PY_FFTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FFTAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define FFTATTRIBUTES_NMETH 4
void           PyFFTAttributes_StartUp(FFTAttributes *subj, void *data);
void           PyFFTAttributes_CloseDown();
PyMethodDef *  PyFFTAttributes_GetMethodTable(int *nMethods);
bool           PyFFTAttributes_Check(PyObject *obj);
FFTAttributes *  PyFFTAttributes_FromPyObject(PyObject *obj);
PyObject *     PyFFTAttributes_New();
PyObject *     PyFFTAttributes_Wrap(const FFTAttributes *attr);
void           PyFFTAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyFFTAttributes_SetDefaults(const FFTAttributes *atts);
std::string    PyFFTAttributes_GetLogString();
std::string    PyFFTAttributes_ToString(const FFTAttributes *, const char *, const bool=false);
PyObject *     PyFFTAttributes_getattr(PyObject *self, char *name);
int            PyFFTAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyFFTAttributes_methods[FFTATTRIBUTES_NMETH];

#endif

