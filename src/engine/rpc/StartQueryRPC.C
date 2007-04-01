#include "StartQueryRPC.h"


// ****************************************************************************
//  Constructor: StartQueryRPC::StartQueryRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

StartQueryRPC::StartQueryRPC() : BlockingRPC("bi")
{
}

// ****************************************************************************
//  Destructor: StartQueryRPC::~StartQueryRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

StartQueryRPC::~StartQueryRPC()
{
}

// ****************************************************************************
//  Method: StartQueryRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::operator()(const bool flag, const int nid)
{
    SetStartFlag(flag);
    SetNetId(nid);

    Execute();
}

// ****************************************************************************
//  Method: StartQueryRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SelectAll()
{
    Select(0, (void*)&startFlag);
    Select(1, (void*)&netId);
}


// ****************************************************************************
//  Method: StartQueryRPC::SetStartFlag
//
//  Purpose:  Set the value of startFlag.
//
//  Arguments:
//    flag      If true, start pick mode.  Stop otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SetStartFlag(const bool flag)
{
    startFlag = flag;
    Select(0, (void*)&startFlag);
}


// ****************************************************************************
//  Method: StartQueryRPC::GetStartFlag
//
//  Purpose: 
//    This returns the flag specifying whether to start or stop pick mode.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

bool
StartQueryRPC::GetStartFlag() 
{
    return startFlag;
}


// ****************************************************************************
//  Method: StartQueryRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(1, (void*)&netId);
}


// ****************************************************************************
//  Method: StartQueryRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

int
StartQueryRPC::GetNetId() 
{
    return netId;
}

