#ifndef PY_COORDSWAPATTRIBUTES_H
#define PY_COORDSWAPATTRIBUTES_H
#include <Python.h>
#include <CoordSwapAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCoordSwapAttributes_StartUp(CoordSwapAttributes *subj, void *data);
void            PyCoordSwapAttributes_CloseDown();
PyMethodDef    *PyCoordSwapAttributes_GetMethodTable(int *nMethods);
bool            PyCoordSwapAttributes_Check(PyObject *obj);
CoordSwapAttributes *PyCoordSwapAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCoordSwapAttributes_NewPyObject();
PyObject       *PyCoordSwapAttributes_WrapPyObject(const CoordSwapAttributes *attr);
std::string     PyCoordSwapAttributes_GetLogString();
void            PyCoordSwapAttributes_SetDefaults(const CoordSwapAttributes *atts);

#endif

