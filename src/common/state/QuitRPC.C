#include <QuitRPC.h>
#include <DebugStream.h>

// ****************************************************************************
//  Constructor: QuitRPC::QuitRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Apr 24 10:26:25 PDT 2003
//    I made it be nonblocking.
//
// ****************************************************************************

QuitRPC::QuitRPC() : NonBlockingRPC("b",NULL)
{
    quit = false;
}

// ****************************************************************************
//  Destructor: QuitRPC::~QuitRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

QuitRPC::~QuitRPC()
{
}

// ****************************************************************************
//  Method: QuitRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    q         the quit state
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:41:39 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// ****************************************************************************

void
QuitRPC::operator()(bool q)
{
    debug3 << "Executing quit RPC" 
           << "\n\t quit='" << q << "'"
           << endl;

    SetQuit(q);
    Execute();
}

// ****************************************************************************
//  Method: QuitRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
QuitRPC::SelectAll()
{
    Select(0, (void*)&quit);
}

// ****************************************************************************
//  Method: QuitRPC::SetQuit
//
//  Purpose: 
//    This sets the quit state.
//
//  Arguments:
//    q         the quit state
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
QuitRPC::SetQuit(bool q)
{
    quit = q;
}

// ****************************************************************************
//  Method: QuitRPC::GetQuit
//
//  Purpose: 
//    This returns the quit state.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

bool
QuitRPC::GetQuit() const
{
    return quit;
}
