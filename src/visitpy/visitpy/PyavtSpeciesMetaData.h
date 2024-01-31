// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSPECIESMETADATA_H
#define PY_AVTSPECIESMETADATA_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtSpeciesMetaData.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSPECIESMETADATA_NMETH 19
void VISITPY_API           PyavtSpeciesMetaData_StartUp(avtSpeciesMetaData *subj, void *data);
void VISITPY_API           PyavtSpeciesMetaData_CloseDown();
VISITPY_API PyMethodDef *  PyavtSpeciesMetaData_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtSpeciesMetaData_Check(PyObject *obj);
VISITPY_API avtSpeciesMetaData *  PyavtSpeciesMetaData_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtSpeciesMetaData_New();
VISITPY_API PyObject *     PyavtSpeciesMetaData_Wrap(const avtSpeciesMetaData *attr);
void VISITPY_API           PyavtSpeciesMetaData_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtSpeciesMetaData_SetDefaults(const avtSpeciesMetaData *atts);
std::string VISITPY_API    PyavtSpeciesMetaData_GetLogString();
std::string VISITPY_API    PyavtSpeciesMetaData_ToString(const avtSpeciesMetaData *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtSpeciesMetaData_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtSpeciesMetaData_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtSpeciesMetaData_methods[AVTSPECIESMETADATA_NMETH];

#endif

