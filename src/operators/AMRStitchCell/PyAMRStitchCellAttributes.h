// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AMRSTITCHCELLATTRIBUTES_H
#define PY_AMRSTITCHCELLATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AMRStitchCellAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define AMRSTITCHCELLATTRIBUTES_NMETH 4
void           PyAMRStitchCellAttributes_StartUp(AMRStitchCellAttributes *subj, void *data);
void           PyAMRStitchCellAttributes_CloseDown();
PyMethodDef *  PyAMRStitchCellAttributes_GetMethodTable(int *nMethods);
bool           PyAMRStitchCellAttributes_Check(PyObject *obj);
AMRStitchCellAttributes *  PyAMRStitchCellAttributes_FromPyObject(PyObject *obj);
PyObject *     PyAMRStitchCellAttributes_New();
PyObject *     PyAMRStitchCellAttributes_Wrap(const AMRStitchCellAttributes *attr);
void           PyAMRStitchCellAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyAMRStitchCellAttributes_SetDefaults(const AMRStitchCellAttributes *atts);
std::string    PyAMRStitchCellAttributes_GetLogString();
std::string    PyAMRStitchCellAttributes_ToString(const AMRStitchCellAttributes *, const char *, const bool=false);
PyObject *     PyAMRStitchCellAttributes_getattr(PyObject *self, char *name);
int            PyAMRStitchCellAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyAMRStitchCellAttributes_methods[AMRSTITCHCELLATTRIBUTES_NMETH];

#endif

