#ifndef PY_CYLINDERATTRIBUTES_H
#define PY_CYLINDERATTRIBUTES_H
#include <Python.h>
#include <CylinderAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCylinderAttributes_StartUp(CylinderAttributes *subj, FILE *logFile);
void            PyCylinderAttributes_CloseDown();
PyMethodDef    *PyCylinderAttributes_GetMethodTable(int *nMethods);
bool            PyCylinderAttributes_Check(PyObject *obj);
CylinderAttributes *PyCylinderAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCylinderAttributes_NewPyObject();
PyObject       *PyCylinderAttributes_WrapPyObject(const CylinderAttributes *attr);
void            PyCylinderAttributes_SetLogging(bool val);
void            PyCylinderAttributes_SetDefaults(const CylinderAttributes *atts);

#endif

