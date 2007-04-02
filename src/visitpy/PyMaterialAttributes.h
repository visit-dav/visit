#ifndef PY_MATERIALATTRIBUTES_H
#define PY_MATERIALATTRIBUTES_H
#include <Python.h>
#include <MaterialAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMaterialAttributes_StartUp(MaterialAttributes *subj, FILE *logFile);
void            PyMaterialAttributes_CloseDown();
PyMethodDef    *PyMaterialAttributes_GetMethodTable(int *nMethods);
bool            PyMaterialAttributes_Check(PyObject *obj);
MaterialAttributes *PyMaterialAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMaterialAttributes_NewPyObject();
PyObject       *PyMaterialAttributes_WrapPyObject(const MaterialAttributes *attr);
void            PyMaterialAttributes_SetLogging(bool val);
void            PyMaterialAttributes_SetDefaults(const MaterialAttributes *atts);

PyObject       *PyMaterialAttributes_StringRepresentation(const MaterialAttributes *atts);

#endif

