#include "SetWindowAttsRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: SetWindowAttsRPC::SetWindowAttsRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

SetWindowAttsRPC::SetWindowAttsRPC() : BlockingRPC("a")
{
}

// ****************************************************************************
//  Destructor: SetWindowAttsRPC::~SetWindowAttsRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

SetWindowAttsRPC::~SetWindowAttsRPC()
{
}

// ****************************************************************************
//  Method: SetWindowAttsRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the window atts
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
//  Modifications:
// ****************************************************************************

void
SetWindowAttsRPC::operator()(const WindowAttributes *atts)
{
    SetWindowAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: SetWindowAttsRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

void
SetWindowAttsRPC::SelectAll()
{
    Select(0, (void*)&win);
}


// ****************************************************************************
//  Method: SetWindowAttsRPC::SetWindowAtts
//
//  Purpose: 
//    This sets the window atts.
//
//  Arguments:
//    atts      the window atts
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

void
SetWindowAttsRPC::SetWindowAtts(const WindowAttributes *atts)
{
    win = *atts;
    Select(0, (void*)&win);
}


// ****************************************************************************
//  Method: SetWindowAttsRPC::GetWindowAtts
//
//  Purpose: 
//    This returns the window atts.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

const WindowAttributes&
SetWindowAttsRPC::GetWindowAtts() const
{
    return win;
}

