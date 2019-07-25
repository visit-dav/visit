// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _RESTORE_SESSION_RPC_EXECUTOR_H
#define _RESTORE_SESSION_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: RestoreSessionRPCExecutor
//
// Purpose:
//   This class handles the RPC request to load the session file from a 
//   remote host.
//
// Notes:      
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

class RestoreSessionRPCExecutor : public Observer
{
public:
    RestoreSessionRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~RestoreSessionRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
