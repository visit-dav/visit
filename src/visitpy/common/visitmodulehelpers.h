// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MODULE_HELPERS_H
#define VISIT_MODULE_HELPERS_H
#include <ViewerProxy.h>
#include <Python.h>
#include <Py2and3Support.h>

//
// Visible Prototypes.
//
void VisItErrorFunc(const char *errString);
ViewerProxy   *GetViewerProxy();
ViewerState   *GetViewerState();
ViewerMethods *GetViewerMethods();


// ---------------------------------------------------------------
//
// Our thread locking solution did not work in Python 3.
// This lead to CLI crashing whenever a lock was requested
// (the primary case was command line recording, but there
//  may be others)
//
// Prior locking solution worked fine in Python 2, so we keep
// it when Python 2 is in play, but provide a new Python 3
// solution using on PyGILState_Ensure() + PyGILState_Release()
//
// ---------------------------------------------------------------

#if defined(IS_PY3K)
// ---------------------------------------------------------------
// Python 3 lock state
// ---------------------------------------------------------------
#define VISIT_PY_THREAD_LOCK_STATE PyGILState_STATE
// ---------------------------------------------------------------
#else
// ---------------------------------------------------------------
// Python 2 lock state
// ---------------------------------------------------------------
#define VISIT_PY_THREAD_LOCK_STATE PyThreadState *
// ---------------------------------------------------------------
#endif 

// Functions from visitmodule.C that lock and release the Python interpreter.
VISIT_PY_THREAD_LOCK_STATE  VisItLockPythonInterpreter();
void VisItUnlockPythonInterpreter(VISIT_PY_THREAD_LOCK_STATE);

#endif
