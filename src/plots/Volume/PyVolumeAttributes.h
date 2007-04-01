#ifndef PY_VOLUMEATTRIBUTES_H
#define PY_VOLUMEATTRIBUTES_H
#include <Python.h>
#include <VolumeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyVolumeAttributes_StartUp(VolumeAttributes *subj, FILE *logFile);
void            PyVolumeAttributes_CloseDown();
PyMethodDef    *PyVolumeAttributes_GetMethodTable(int *nMethods);
bool            PyVolumeAttributes_Check(PyObject *obj);
VolumeAttributes *PyVolumeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyVolumeAttributes_NewPyObject();
PyObject       *PyVolumeAttributes_WrapPyObject(const VolumeAttributes *attr);
void            PyVolumeAttributes_SetLogging(bool val);
void            PyVolumeAttributes_SetDefaults(const VolumeAttributes *atts);

PyObject       *PyVolumeAttributes_StringRepresentation(const VolumeAttributes *atts);

#endif

