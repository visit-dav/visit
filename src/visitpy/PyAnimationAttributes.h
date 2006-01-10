#ifndef PY_ANIMATIONATTRIBUTES_H
#define PY_ANIMATIONATTRIBUTES_H
#include <Python.h>
#include <AnimationAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyAnimationAttributes_StartUp(AnimationAttributes *subj, void *data);
void            PyAnimationAttributes_CloseDown();
PyMethodDef    *PyAnimationAttributes_GetMethodTable(int *nMethods);
bool            PyAnimationAttributes_Check(PyObject *obj);
AnimationAttributes *PyAnimationAttributes_FromPyObject(PyObject *obj);
PyObject       *PyAnimationAttributes_NewPyObject();
PyObject       *PyAnimationAttributes_WrapPyObject(const AnimationAttributes *attr);
void            PyAnimationAttributes_SetDefaults(const AnimationAttributes *atts);
std::string     PyAnimationAttributes_GetLogString();
std::string     PyAnimationAttributes_ToString(const AnimationAttributes *, const char *);

#endif

