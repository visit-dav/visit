#ifndef PY_VOLUMEATTRIBUTES_H
#define PY_VOLUMEATTRIBUTES_H
#include <Python.h>
#include <VolumeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyVolumeAttributes_StartUp(VolumeAttributes *subj, void *data);
void            PyVolumeAttributes_CloseDown();
PyMethodDef    *PyVolumeAttributes_GetMethodTable(int *nMethods);
bool            PyVolumeAttributes_Check(PyObject *obj);
VolumeAttributes *PyVolumeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyVolumeAttributes_NewPyObject();
PyObject       *PyVolumeAttributes_WrapPyObject(const VolumeAttributes *attr);
std::string     PyVolumeAttributes_GetLogString();
void            PyVolumeAttributes_SetDefaults(const VolumeAttributes *atts);

#endif

