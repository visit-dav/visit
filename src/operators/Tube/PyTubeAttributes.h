#ifndef PY_TUBEATTRIBUTES_H
#define PY_TUBEATTRIBUTES_H
#include <Python.h>
#include <TubeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTubeAttributes_StartUp(TubeAttributes *subj, FILE *logFile);
void            PyTubeAttributes_CloseDown();
PyMethodDef    *PyTubeAttributes_GetMethodTable(int *nMethods);
bool            PyTubeAttributes_Check(PyObject *obj);
TubeAttributes *PyTubeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTubeAttributes_NewPyObject();
PyObject       *PyTubeAttributes_WrapPyObject(const TubeAttributes *attr);
void            PyTubeAttributes_SetLogging(bool val);
void            PyTubeAttributes_SetDefaults(const TubeAttributes *atts);

#endif

