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

private:
    bool quit;
};

#endif
