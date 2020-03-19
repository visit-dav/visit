// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CALLBACK_HANDLERS_H
#define CALLBACK_HANDLERS_H
#include <CallbackManager.h>
#include <ViewerProxy.h>
#include <ViewerRPCCallbacks.h>

//
// This function registers callback handlers for the supported state objects.
//
void RegisterCallbackHandlers(CallbackManager *cb, ViewerProxy *viewer, ViewerRPCCallbacks *rpcCB);
PyObject *GetPyObjectPluginAttributes(AttributeSubject *subj, bool useCurrent, ViewerProxy *);

#endif
