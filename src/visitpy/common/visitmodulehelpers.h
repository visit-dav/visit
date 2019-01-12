#ifndef VISIT_MODULE_HELPERS_H
#define VISIT_MODULE_HELPERS_H
#include <ViewerProxy.h>
#include <Python.h>

//
// Visible Prototypes.
//
void VisItErrorFunc(const char *errString);
ViewerProxy   *GetViewerProxy();
ViewerState   *GetViewerState();
ViewerMethods *GetViewerMethods();

// Functions from visitmodule.C that lock and release the Python interpreter.
PyThreadState *VisItLockPythonInterpreter();
void VisItUnlockPythonInterpreter(PyThreadState *);

#endif
