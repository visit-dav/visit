// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTMATSPECIESMETADATA_H
#define PY_AVTMATSPECIESMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtMatSpeciesMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTMATSPECIESMETADATA_NMETH 8
void VISITPY_API           PyavtMatSpeciesMetaData_StartUp(avtMatSpeciesMetaData *subj, void *data);
void VISITPY_API           PyavtMatSpeciesMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtMatSpeciesMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtMatSpeciesMetaData_Check(PyObject *obj);
VISITPY_API avtMatSpeciesMetaData *  PyavtMatSpeciesMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtMatSpeciesMetaData_New();
VISITPY_API PyObject *     PyavtMatSpeciesMetaData_Wrap(const avtMatSpeciesMetaData *attr);
void VISITPY_API           PyavtMatSpeciesMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtMatSpeciesMetaData_SetDefaults(const avtMatSpeciesMetaData *atts);
std::string VISITPY_API    PyavtMatSpeciesMetaData_GetLogString();
std::string VISITPY_API    PyavtMatSpeciesMetaData_ToString(const avtMatSpeciesMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtMatSpeciesMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtMatSpeciesMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtMatSpeciesMetaData_methods[AVTMATSPECIESMETADATA_NMETH];

#endif

