// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSIMULATIONINFORMATION_H
#define PY_AVTSIMULATIONINFORMATION_H
#include <Python.h>
#include <avtSimulationInformation.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSIMULATIONINFORMATION_NMETH 26
void VISITPY_API           PyavtSimulationInformation_StartUp(avtSimulationInformation *subj, void *data);
void VISITPY_API           PyavtSimulationInformation_CloseDown();
VISITPY_API PyMethodDef *  PyavtSimulationInformation_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtSimulationInformation_Check(PyObject *obj);
VISITPY_API avtSimulationInformation *  PyavtSimulationInformation_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtSimulationInformation_New();
VISITPY_API PyObject *     PyavtSimulationInformation_Wrap(const avtSimulationInformation *attr);
void VISITPY_API           PyavtSimulationInformation_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtSimulationInformation_SetDefaults(const avtSimulationInformation *atts);
std::string VISITPY_API    PyavtSimulationInformation_GetLogString();
std::string VISITPY_API    PyavtSimulationInformation_ToString(const avtSimulationInformation *, const char *);
VISITPY_API PyObject *     PyavtSimulationInformation_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtSimulationInformation_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtSimulationInformation_methods[AVTSIMULATIONINFORMATION_NMETH];

#endif

