// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CHANGE_DIRECTORY_RPC_EXECUTOR_H
#define CHANGE_DIRECTORY_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: ChangeDirectoryRPCExecutor
//
// Purpose:
//   This class handles the RPC request for changing the current
//   working directory.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

class ChangeDirectoryRPCExecutor : public Observer
{
public:
    ChangeDirectoryRPCExecutor(MDServerConnection *parent_, Subject *s);
    ~ChangeDirectoryRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
