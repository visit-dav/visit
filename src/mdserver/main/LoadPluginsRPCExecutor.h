// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LOAD_PLUGINS_RPC_EXECUTOR_H
#define LOAD_PLUGINS_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: LoadPluginsRPCExecutor
//
// Purpose:
//   This class handles the RPC request to load the plugins.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

class LoadPluginsRPCExecutor : public Observer
{
public:
    LoadPluginsRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~LoadPluginsRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
