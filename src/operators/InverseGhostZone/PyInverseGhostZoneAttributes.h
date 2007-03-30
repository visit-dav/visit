#ifndef PY_INVERSEGHOSTZONEATTRIBUTES_H
#define PY_INVERSEGHOSTZONEATTRIBUTES_H
#include <Python.h>
#include <InverseGhostZoneAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyInverseGhostZoneAttributes_StartUp(InverseGhostZoneAttributes *subj, FILE *logFile);
void            PyInverseGhostZoneAttributes_CloseDown();
PyMethodDef    *PyInverseGhostZoneAttributes_GetMethodTable(int *nMethods);
bool            PyInverseGhostZoneAttributes_Check(PyObject *obj);
InverseGhostZoneAttributes *PyInverseGhostZoneAttributes_FromPyObject(PyObject *obj);
PyObject       *PyInverseGhostZoneAttributes_NewPyObject();
PyObject       *PyInverseGhostZoneAttributes_WrapPyObject(const InverseGhostZoneAttributes *attr);
void            PyInverseGhostZoneAttributes_SetLogging(bool val);
void            PyInverseGhostZoneAttributes_SetDefaults(const InverseGhostZoneAttributes *atts);

#endif

