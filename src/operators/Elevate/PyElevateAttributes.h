#ifndef PY_ELEVATEATTRIBUTES_H
#define PY_ELEVATEATTRIBUTES_H
#include <Python.h>
#include <ElevateAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyElevateAttributes_StartUp(ElevateAttributes *subj, FILE *logFile);
void            PyElevateAttributes_CloseDown();
PyMethodDef    *PyElevateAttributes_GetMethodTable(int *nMethods);
bool            PyElevateAttributes_Check(PyObject *obj);
ElevateAttributes *PyElevateAttributes_FromPyObject(PyObject *obj);
PyObject       *PyElevateAttributes_NewPyObject();
PyObject       *PyElevateAttributes_WrapPyObject(const ElevateAttributes *attr);
void            PyElevateAttributes_SetLogging(bool val);
void            PyElevateAttributes_SetDefaults(const ElevateAttributes *atts);

PyObject       *PyElevateAttributes_StringRepresentation(const ElevateAttributes *atts);

#endif

