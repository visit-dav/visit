// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PERSISTENTPARTICLESATTRIBUTES_H
#define PY_PERSISTENTPARTICLESATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PersistentParticlesAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PERSISTENTPARTICLESATTRIBUTES_NMETH 24
void           PyPersistentParticlesAttributes_StartUp(PersistentParticlesAttributes *subj, void *data);
void           PyPersistentParticlesAttributes_CloseDown();
PyMethodDef *  PyPersistentParticlesAttributes_GetMethodTable(int *nMethods);
bool           PyPersistentParticlesAttributes_Check(PyObject *obj);
PersistentParticlesAttributes *  PyPersistentParticlesAttributes_FromPyObject(PyObject *obj);
PyObject *     PyPersistentParticlesAttributes_New();
PyObject *     PyPersistentParticlesAttributes_Wrap(const PersistentParticlesAttributes *attr);
void           PyPersistentParticlesAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyPersistentParticlesAttributes_SetDefaults(const PersistentParticlesAttributes *atts);
std::string    PyPersistentParticlesAttributes_GetLogString();
std::string    PyPersistentParticlesAttributes_ToString(const PersistentParticlesAttributes *, const char *, const bool=false);
PyObject *     PyPersistentParticlesAttributes_getattr(PyObject *self, char *name);
int            PyPersistentParticlesAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyPersistentParticlesAttributes_methods[PERSISTENTPARTICLESATTRIBUTES_NMETH];

#endif

