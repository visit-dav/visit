// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COORDSWAPATTRIBUTES_H
#define PY_COORDSWAPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CoordSwapAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define COORDSWAPATTRIBUTES_NMETH 8
void           PyCoordSwapAttributes_StartUp(CoordSwapAttributes *subj, void *data);
void           PyCoordSwapAttributes_CloseDown();
PyMethodDef *  PyCoordSwapAttributes_GetMethodTable(int *nMethods);
bool           PyCoordSwapAttributes_Check(PyObject *obj);
CoordSwapAttributes *  PyCoordSwapAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCoordSwapAttributes_New();
PyObject *     PyCoordSwapAttributes_Wrap(const CoordSwapAttributes *attr);
void           PyCoordSwapAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCoordSwapAttributes_SetDefaults(const CoordSwapAttributes *atts);
std::string    PyCoordSwapAttributes_GetLogString();
std::string    PyCoordSwapAttributes_ToString(const CoordSwapAttributes *, const char *, const bool=false);
PyObject *     PyCoordSwapAttributes_getattr(PyObject *self, char *name);
int            PyCoordSwapAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCoordSwapAttributes_methods[COORDSWAPATTRIBUTES_NMETH];

#endif

