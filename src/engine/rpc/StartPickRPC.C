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
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added another bool arg.
//
// ****************************************************************************

StartPickRPC::StartPickRPC() : BlockingRPC("bbi")
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
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added bool forZones arg.
//
// ****************************************************************************

void
StartPickRPC::operator()(const bool forZones, const bool flag, const int nid)
{
    SetForZones(forZones);
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
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added forZones.
//
// ****************************************************************************

void
StartPickRPC::SelectAll()
{
    Select(0, (void*)&forZones);
    Select(1, (void*)&startFlag);
    Select(2, (void*)&netId);
}


// ****************************************************************************
//  Method: StartPickRPC::SetForZones
//
//  Purpose:  Set the value of forZones.
//
//  Arguments:
//    forZones  If true, pick mode is for zone picking, otherwise pick mode
//              is for node picking.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
StartPickRPC::SetForZones(const bool forZonesFlag)
{
    forZones = forZonesFlag;
    Select(0, (void*)&forZones);
}


// ****************************************************************************
//  Method: StartPickRPC::GetForZones
//
//  Purpose: 
//    This returns the flag specifying picking is for zones or nodes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

bool
StartPickRPC::GetForZones() 
{
    return forZones;
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
    Select(1, (void*)&startFlag);
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
    Select(2, (void*)&netId);
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

