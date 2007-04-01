#ifndef PY_SUBDIVIDEQUADSATTRIBUTES_H
#define PY_SUBDIVIDEQUADSATTRIBUTES_H
#include <Python.h>
#include <SubdivideQuadsAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySubdivideQuadsAttributes_StartUp(SubdivideQuadsAttributes *subj, FILE *logFile);
void            PySubdivideQuadsAttributes_CloseDown();
PyMethodDef    *PySubdivideQuadsAttributes_GetMethodTable(int *nMethods);
bool            PySubdivideQuadsAttributes_Check(PyObject *obj);
SubdivideQuadsAttributes *PySubdivideQuadsAttributes_FromPyObject(PyObject *obj);
PyObject       *PySubdivideQuadsAttributes_NewPyObject();
PyObject       *PySubdivideQuadsAttributes_WrapPyObject(const SubdivideQuadsAttributes *attr);
void            PySubdivideQuadsAttributes_SetLogging(bool val);
void            PySubdivideQuadsAttributes_SetDefaults(const SubdivideQuadsAttributes *atts);

PyObject       *PySubdivideQuadsAttributes_StringRepresentation(const SubdivideQuadsAttributes *atts);

#endif

