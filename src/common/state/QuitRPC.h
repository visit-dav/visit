// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUIT_RPC_H
#define QUIT_RPC_H
#include <state_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: QuitRPC
//
// Purpose:
//   This is an RPC that tells a program to quit.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 15:50:11 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 10:23:39 PDT 2003
//   I made it be nonblocking.
//
//   Brad Whitlock, Fri Dec  7 11:52:21 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class STATE_API QuitRPC : public NonBlockingRPC
{
public:
    QuitRPC();
    virtual ~QuitRPC();

    // Invokation method
    void operator()(bool);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetQuit(bool);

    // Property getting methods
    bool GetQuit() const;

    virtual const std::string TypeName() const;
private:
    bool quit;
};

#endif
