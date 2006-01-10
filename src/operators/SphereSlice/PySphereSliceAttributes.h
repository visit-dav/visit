#ifndef PY_SPHERESLICEATTRIBUTES_H
#define PY_SPHERESLICEATTRIBUTES_H
#include <Python.h>
#include <SphereSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySphereSliceAttributes_StartUp(SphereSliceAttributes *subj, void *data);
void            PySphereSliceAttributes_CloseDown();
PyMethodDef    *PySphereSliceAttributes_GetMethodTable(int *nMethods);
bool            PySphereSliceAttributes_Check(PyObject *obj);
SphereSliceAttributes *PySphereSliceAttributes_FromPyObject(PyObject *obj);
PyObject       *PySphereSliceAttributes_NewPyObject();
PyObject       *PySphereSliceAttributes_WrapPyObject(const SphereSliceAttributes *attr);
std::string     PySphereSliceAttributes_GetLogString();
void            PySphereSliceAttributes_SetDefaults(const SphereSliceAttributes *atts);

#endif

