#include "StartPickRPC.h"


// ****************************************************************************
//  Constructor: StartPickRPC::StartPickRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

StartPickRPC::StartPickRPC() : BlockingRPC("bi")
{
}

// ****************************************************************************
//  Destructor: StartPickRPC::~StartPickRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

StartPickRPC::~StartPickRPC()
{
}

// ****************************************************************************
//  Method: StartPickRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
//  Modifications:
// ****************************************************************************

void
StartPickRPC::operator()(const bool flag, const int nid)
{
    SetStartFlag(flag);
    SetNetId(nid);

    Execute();
}

// ****************************************************************************
//  Method: StartPickRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

void
StartPickRPC::SelectAll()
{
    Select(0, (void*)&startFlag);
    Select(1, (void*)&netId);
}

// ****************************************************************************
//  Method: StartPickRPC::SetStartFlag
//
//  Purpose:  Set the value of startFlag.
//
//  Arguments:
//    flag      If true, start pick mode.  Stop otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

void
StartPickRPC::SetStartFlag(const bool flag)
{
    startFlag = flag;
    Select(0, (void*)&startFlag);
}


// ****************************************************************************
//  Method: StartPickRPC::GetStartFlag
//
//  Purpose: 
//    This returns the flag specifying whether to start or stop pick mode.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

bool
StartPickRPC::GetStartFlag() 
{
    return startFlag;
}


// ****************************************************************************
//  Method: StartPickRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

void
StartPickRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(1, (void*)&netId);
}


// ****************************************************************************
//  Method: StartPickRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

int
StartPickRPC::GetNetId() 
{
    return netId;
}

