#ifndef PY_TOPOLOGYATTRIBUTES_H
#define PY_TOPOLOGYATTRIBUTES_H
#include <Python.h>
#include <TopologyAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTopologyAttributes_StartUp(TopologyAttributes *subj, FILE *logFile);
void            PyTopologyAttributes_CloseDown();
PyMethodDef    *PyTopologyAttributes_GetMethodTable(int *nMethods);
bool            PyTopologyAttributes_Check(PyObject *obj);
TopologyAttributes *PyTopologyAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTopologyAttributes_NewPyObject();
PyObject       *PyTopologyAttributes_WrapPyObject(const TopologyAttributes *attr);
void            PyTopologyAttributes_SetLogging(bool val);
void            PyTopologyAttributes_SetDefaults(const TopologyAttributes *atts);

#endif

