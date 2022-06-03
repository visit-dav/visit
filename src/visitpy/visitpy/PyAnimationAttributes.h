// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ANIMATIONATTRIBUTES_H
#define PY_ANIMATIONATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AnimationAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define ANIMATIONATTRIBUTES_NMETH 12
void VISITPY_API           PyAnimationAttributes_StartUp(AnimationAttributes *subj, void *data);
void VISITPY_API           PyAnimationAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyAnimationAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAnimationAttributes_Check(PyObject *obj);
VISITPY_API AnimationAttributes *  PyAnimationAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAnimationAttributes_New();
VISITPY_API PyObject *     PyAnimationAttributes_Wrap(const AnimationAttributes *attr);
void VISITPY_API           PyAnimationAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAnimationAttributes_SetDefaults(const AnimationAttributes *atts);
std::string VISITPY_API    PyAnimationAttributes_GetLogString();
std::string VISITPY_API    PyAnimationAttributes_ToString(const AnimationAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyAnimationAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyAnimationAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAnimationAttributes_methods[ANIMATIONATTRIBUTES_NMETH];

#endif

