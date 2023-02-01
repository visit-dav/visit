// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_REPLICATEATTRIBUTES_H
#define PY_REPLICATEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ReplicateAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define REPLICATEATTRIBUTES_NMETH 24
void           PyReplicateAttributes_StartUp(ReplicateAttributes *subj, void *data);
void           PyReplicateAttributes_CloseDown();
PyMethodDef *  PyReplicateAttributes_GetMethodTable(int *nMethods);
bool           PyReplicateAttributes_Check(PyObject *obj);
ReplicateAttributes *  PyReplicateAttributes_FromPyObject(PyObject *obj);
PyObject *     PyReplicateAttributes_New();
PyObject *     PyReplicateAttributes_Wrap(const ReplicateAttributes *attr);
void           PyReplicateAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyReplicateAttributes_SetDefaults(const ReplicateAttributes *atts);
std::string    PyReplicateAttributes_GetLogString();
std::string    PyReplicateAttributes_ToString(const ReplicateAttributes *, const char *, const bool=false);
PyObject *     PyReplicateAttributes_getattr(PyObject *self, char *name);
int            PyReplicateAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyReplicateAttributes_methods[REPLICATEATTRIBUTES_NMETH];

#endif

