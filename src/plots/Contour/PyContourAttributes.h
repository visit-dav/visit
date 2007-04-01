#ifndef PY_CONTOURATTRIBUTES_H
#define PY_CONTOURATTRIBUTES_H
#include <Python.h>
#include <ContourAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyContourAttributes_StartUp(ContourAttributes *subj, FILE *logFile);
void            PyContourAttributes_CloseDown();
PyMethodDef    *PyContourAttributes_GetMethodTable(int *nMethods);
bool            PyContourAttributes_Check(PyObject *obj);
ContourAttributes *PyContourAttributes_FromPyObject(PyObject *obj);
PyObject       *PyContourAttributes_NewPyObject();
PyObject       *PyContourAttributes_WrapPyObject(const ContourAttributes *attr);
void            PyContourAttributes_SetLogging(bool val);
void            PyContourAttributes_SetDefaults(const ContourAttributes *atts);

PyObject       *PyContourAttributes_StringRepresentation(const ContourAttributes *atts);

#endif

