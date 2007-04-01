#include "PickRPC.h"


// ****************************************************************************
//  Constructor: PickRPC::PickRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

PickRPC::PickRPC() : BlockingRPC("iai", &returnAtts)
{
}

// ****************************************************************************
//  Destructor: PickRPC::~PickRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

PickRPC::~PickRPC()
{
}

// ****************************************************************************
//  Method: PickRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

PickAttributes
PickRPC::operator()(const int nid, const PickAttributes *atts, int wid)
{
    SetNetId(nid);
    SetPickAtts(atts);
    SetWinId(wid);
    Execute();

    return returnAtts;
}

// ****************************************************************************
//  Method: PickRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
// ****************************************************************************

void
PickRPC::SelectAll()
{
    Select(0, (void*)&netId);
    Select(1, (void*)&pickAtts);
    Select(2, (void*)&winId);
}


// ****************************************************************************
//  Method: PickRPC::SetPickAtts
//
//  Purpose: 
//    This sets the pickAttsdow atts.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
PickRPC::SetPickAtts(const PickAttributes *atts)
{
    pickAtts = *atts;
    Select(1, (void*)&pickAtts);
}


// ****************************************************************************
//  Method: PickRPC::GetPickAtts
//
//  Purpose: 
//    This returns the pickAttsdow atts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

PickAttributes*
PickRPC::GetPickAtts() 
{
    return &pickAtts;
}


// ****************************************************************************
//  Method: PickRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
PickRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(0, (void*)&netId);
}

// ****************************************************************************
//  Method: PickRPC::SetWinId
//
//  Programmer: Mark C. Miller 
//  Creation:   December 15, 2004 
//
// ****************************************************************************
void
PickRPC::SetWinId(const int wid)
{
    winId = wid;
    Select(2, (void*)&winId);
}


// ****************************************************************************
//  Method: PickRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

int
PickRPC::GetNetId() 
{
    return netId;
}

// ****************************************************************************
//  Method: PickRPC::SetWinId
//
//  Programmer: Mark C. Miller 
//  Creation:   December 15, 2004 
//
// ****************************************************************************
int
PickRPC::GetWinId() const
{
    return winId;
}
