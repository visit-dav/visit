#ifndef PY_COORDSWAP_H
#define PY_COORDSWAP_H
#include <Python.h>
#include <CoordSwap.h>

//
// Functions exposed to the VisIt module.
//
void            PyCoordSwap_StartUp(CoordSwap *subj, FILE *logFile);
void            PyCoordSwap_CloseDown();
PyMethodDef    *PyCoordSwap_GetMethodTable(int *nMethods);
bool            PyCoordSwap_Check(PyObject *obj);
CoordSwap *PyCoordSwap_FromPyObject(PyObject *obj);
PyObject       *PyCoordSwap_NewPyObject();
PyObject       *PyCoordSwap_WrapPyObject(const CoordSwap *attr);
void            PyCoordSwap_SetLogging(bool val);
void            PyCoordSwap_SetDefaults(const CoordSwap *atts);

PyObject       *PyCoordSwap_StringRepresentation(const CoordSwap *atts);

#endif

