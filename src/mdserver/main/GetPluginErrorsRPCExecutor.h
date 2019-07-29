// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GET_PLUGIN_ERRORS_RPC_EXECUTOR_H
#define GET_PLUGIN_ERRORS_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: GetPluginErrorsRPCExecutor
//
// Purpose:
//   This class handles the RPC request to return the errors generated
//   during plugin initialization.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// ****************************************************************************

class GetPluginErrorsRPCExecutor : public Observer
{
public:
    GetPluginErrorsRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~GetPluginErrorsRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
