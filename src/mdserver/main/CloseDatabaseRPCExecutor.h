// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLOSE_DATABASE_RPC_EXECUTOR_H
#define CLOSE_DATABASE_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: CloseDatabaseRPCExecutor
//
// Purpose:
//   This class handles the RPC request to close the active database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 11:04:50 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class CloseDatabaseRPCExecutor : public Observer
{
public:
    CloseDatabaseRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~CloseDatabaseRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
