// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MOLECULEATTRIBUTES_H
#define PY_MOLECULEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MoleculeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define MOLECULEATTRIBUTES_NMETH 44
void           PyMoleculeAttributes_StartUp(MoleculeAttributes *subj, void *data);
void           PyMoleculeAttributes_CloseDown();
PyMethodDef *  PyMoleculeAttributes_GetMethodTable(int *nMethods);
bool           PyMoleculeAttributes_Check(PyObject *obj);
MoleculeAttributes *  PyMoleculeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMoleculeAttributes_New();
PyObject *     PyMoleculeAttributes_Wrap(const MoleculeAttributes *attr);
void           PyMoleculeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMoleculeAttributes_SetDefaults(const MoleculeAttributes *atts);
std::string    PyMoleculeAttributes_GetLogString();
std::string    PyMoleculeAttributes_ToString(const MoleculeAttributes *, const char *, const bool=false);
PyObject *     PyMoleculeAttributes_getattr(PyObject *self, char *name);
int            PyMoleculeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMoleculeAttributes_methods[MOLECULEATTRIBUTES_NMETH];

#endif

