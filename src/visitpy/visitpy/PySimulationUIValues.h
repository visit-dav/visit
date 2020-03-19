// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SIMULATIONUIVALUES_H
#define PY_SIMULATIONUIVALUES_H
#include <Python.h>
#include <SimulationUIValues.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SIMULATIONUIVALUES_NMETH 14
void VISITPY_API           PySimulationUIValues_StartUp(SimulationUIValues *subj, void *data);
void VISITPY_API           PySimulationUIValues_CloseDown();
VISITPY_API PyMethodDef *  PySimulationUIValues_GetMethodTable(int *nMethods);
bool VISITPY_API           PySimulationUIValues_Check(PyObject *obj);
VISITPY_API SimulationUIValues *  PySimulationUIValues_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySimulationUIValues_New();
VISITPY_API PyObject *     PySimulationUIValues_Wrap(const SimulationUIValues *attr);
void VISITPY_API           PySimulationUIValues_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySimulationUIValues_SetDefaults(const SimulationUIValues *atts);
std::string VISITPY_API    PySimulationUIValues_GetLogString();
std::string VISITPY_API    PySimulationUIValues_ToString(const SimulationUIValues *, const char *);
VISITPY_API PyObject *     PySimulationUIValues_getattr(PyObject *self, char *name);
int VISITPY_API            PySimulationUIValues_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySimulationUIValues_methods[SIMULATIONUIVALUES_NMETH];

#endif

