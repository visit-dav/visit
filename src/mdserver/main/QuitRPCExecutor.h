// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUIT_RPC_EXECUTOR_H
#define QUIT_RPC_EXECUTOR_H
#include <Observer.h>

// ****************************************************************************
// Class: QuitRPCExecutor
//
// Purpose:
//   This class handles the RPC request for causing the MDServer to quit.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//
// ****************************************************************************

class QuitRPCExecutor : public Observer
{
public:
    QuitRPCExecutor(Subject *s);
    virtual ~QuitRPCExecutor();

    virtual void Update(Subject *s);
};

#endif
