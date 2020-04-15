// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MODULE_HELPERS_H
#define VISIT_MODULE_HELPERS_H
#include <ViewerProxy.h>
#include <Python.h>
#include "Py2and3Support.h"

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
