#ifdef DEBUG
#include <iostream.h>
#endif
#include <QuitRPCExecutor.h>
#include <QuitRPC.h>

// ****************************************************************************
// Method: QuitRPCExecutor::QuitRPCExecutor
//
// Purpose: 
//   Constructor for the QuitRPCExecutor class.
//
// Arguments:
//   s : A pointer to the QuitRPC object that this executor observes.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

QuitRPCExecutor::QuitRPCExecutor(Subject *s) : Observer(s)
{
}

// ****************************************************************************
// Method: QuitRPCExecutor::~QuitRPCExecutor
//
// Purpose: 
//   Destructor for the QuitRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:04:03 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QuitRPCExecutor::~QuitRPCExecutor()
{
}

// ****************************************************************************
// Method: QuitRPCExecutor::Update
//
// Purpose: 
//   Does the work for the QuitRPC.
//
// Arguments:
//   s : A pointer to the QuitRPC that caused this method to be called.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:05:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QuitRPCExecutor::Update(Subject *s)
{
    QuitRPC *quit = (QuitRPC *)s;
#ifdef DEBUG
    cout << "QuitRPCExecutor::Update\n"; cout.flush();
#endif
    if(!quit->GetQuit())
        quit->SendError();
    else
        quit->SendReply();
}
