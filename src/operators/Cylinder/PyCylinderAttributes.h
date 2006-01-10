#ifndef PY_CYLINDERATTRIBUTES_H
#define PY_CYLINDERATTRIBUTES_H
#include <Python.h>
#include <CylinderAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCylinderAttributes_StartUp(CylinderAttributes *subj, void *data);
void            PyCylinderAttributes_CloseDown();
PyMethodDef    *PyCylinderAttributes_GetMethodTable(int *nMethods);
bool            PyCylinderAttributes_Check(PyObject *obj);
CylinderAttributes *PyCylinderAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCylinderAttributes_NewPyObject();
PyObject       *PyCylinderAttributes_WrapPyObject(const CylinderAttributes *attr);
std::string     PyCylinderAttributes_GetLogString();
void            PyCylinderAttributes_SetDefaults(const CylinderAttributes *atts);

#endif

