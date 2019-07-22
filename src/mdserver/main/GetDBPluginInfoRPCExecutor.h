// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GET_DB_PLUGIN_INFO_RPC_EXECUTOR_H
#define GET_DB_PLUGIN_INFO_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// *******************************************************************
// Class: GetDBPluginInfoRPCExecutor
//
// Purpose:
//   This class handles the RPC request for getting the DBPluginInfo.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// *******************************************************************

class GetDBPluginInfoRPCExecutor : public Observer
{
public:
    GetDBPluginInfoRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~GetDBPluginInfoRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
