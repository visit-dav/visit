// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SAVE_SESSION_RPC_EXECUTOR_H
#define SAVE_SESSION_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: SaveSessionRPCExecutor
//
// Purpose:
//   This class handles the RPC request to save a session file on a remote host.
//
// Notes:      
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

class SaveSessionRPCExecutor : public Observer
{
public:
    SaveSessionRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~SaveSessionRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
