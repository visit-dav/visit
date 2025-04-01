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

#endif

