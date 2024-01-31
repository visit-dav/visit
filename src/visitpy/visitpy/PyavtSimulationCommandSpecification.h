// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AVTSIMULATIONCOMMANDSPECIFICATION_H
#define PY_AVTSIMULATIONCOMMANDSPECIFICATION_H
#include <Python.h>
#include <Py2and3Support.h>
#include <avtSimulationCommandSpecification.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AVTSIMULATIONCOMMANDSPECIFICATION_NMETH 22
void VISITPY_API           PyavtSimulationCommandSpecification_StartUp(avtSimulationCommandSpecification *subj, void *data);
void VISITPY_API           PyavtSimulationCommandSpecification_CloseDown();
VISITPY_API PyMethodDef *  PyavtSimulationCommandSpecification_GetMethodTable(int *nMethods);
bool VISITPY_API           PyavtSimulationCommandSpecification_Check(PyObject *obj);
VISITPY_API avtSimulationCommandSpecification *  PyavtSimulationCommandSpecification_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyavtSimulationCommandSpecification_New();
VISITPY_API PyObject *     PyavtSimulationCommandSpecification_Wrap(const avtSimulationCommandSpecification *attr);
void VISITPY_API           PyavtSimulationCommandSpecification_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyavtSimulationCommandSpecification_SetDefaults(const avtSimulationCommandSpecification *atts);
std::string VISITPY_API    PyavtSimulationCommandSpecification_GetLogString();
std::string VISITPY_API    PyavtSimulationCommandSpecification_ToString(const avtSimulationCommandSpecification *, const char *, const bool=false);
VISITPY_API PyObject *     PyavtSimulationCommandSpecification_getattr(PyObject *self, char *name);
int VISITPY_API            PyavtSimulationCommandSpecification_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyavtSimulationCommandSpecification_methods[AVTSIMULATIONCOMMANDSPECIFICATION_NMETH];

#endif

