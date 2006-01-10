#ifndef PY_DECIMATEATTRIBUTES_H
#define PY_DECIMATEATTRIBUTES_H
#include <Python.h>
#include <DecimateAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyDecimateAttributes_StartUp(DecimateAttributes *subj, void *data);
void            PyDecimateAttributes_CloseDown();
PyMethodDef    *PyDecimateAttributes_GetMethodTable(int *nMethods);
bool            PyDecimateAttributes_Check(PyObject *obj);
DecimateAttributes *PyDecimateAttributes_FromPyObject(PyObject *obj);
PyObject       *PyDecimateAttributes_NewPyObject();
PyObject       *PyDecimateAttributes_WrapPyObject(const DecimateAttributes *attr);
std::string     PyDecimateAttributes_GetLogString();
void            PyDecimateAttributes_SetDefaults(const DecimateAttributes *atts);

#endif

