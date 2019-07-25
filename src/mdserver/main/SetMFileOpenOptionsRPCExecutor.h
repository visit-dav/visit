// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_DEFAULT_FILE_OPEN_OPTIONS_RPC_EXECUTOR_H
#define SET_DEFAULT_FILE_OPEN_OPTIONS_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: SetMFileOpenOptionsRPCExecutor
//
// Purpose:
//   This class handles the RPC request for setting new file opening options.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// ****************************************************************************

class SetMFileOpenOptionsRPCExecutor : public Observer
{
public:
    SetMFileOpenOptionsRPCExecutor(MDServerConnection *, Subject *);
    virtual ~SetMFileOpenOptionsRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
