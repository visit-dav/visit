#ifndef PY_ISOVOLUMEATTRIBUTES_H
#define PY_ISOVOLUMEATTRIBUTES_H
#include <Python.h>
#include <IsovolumeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyIsovolumeAttributes_StartUp(IsovolumeAttributes *subj, FILE *logFile);
void            PyIsovolumeAttributes_CloseDown();
PyMethodDef    *PyIsovolumeAttributes_GetMethodTable(int *nMethods);
bool            PyIsovolumeAttributes_Check(PyObject *obj);
IsovolumeAttributes *PyIsovolumeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyIsovolumeAttributes_NewPyObject();
PyObject       *PyIsovolumeAttributes_WrapPyObject(const IsovolumeAttributes *attr);
void            PyIsovolumeAttributes_SetLogging(bool val);
void            PyIsovolumeAttributes_SetDefaults(const IsovolumeAttributes *atts);

#endif

