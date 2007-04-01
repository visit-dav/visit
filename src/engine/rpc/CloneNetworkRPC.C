#include "CloneNetworkRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: CloneNetworkRPC::CloneNetworkRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
// ****************************************************************************

CloneNetworkRPC::CloneNetworkRPC() : BlockingRPC("ia")
{
}

// ****************************************************************************
//  Destructor: CloneNetworkRPC::~CloneNetworkRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
// ****************************************************************************

CloneNetworkRPC::~CloneNetworkRPC()
{
}

// ****************************************************************************
//  Method: CloneNetworkRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//    origData  the type of input 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
//  Modifications:
// ****************************************************************************

void
CloneNetworkRPC::operator()(const int id_, const QueryOverTimeAttributes *atts)
{
    SetID(id_);
    SetQueryOverTimeAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: CloneNetworkRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
// ****************************************************************************

void
CloneNetworkRPC::SelectAll()
{
    Select(0, (void*)&id);
    Select(1, (void*)&queryAtts);
}


// ****************************************************************************
//  Method: CloneNetworkRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
// ****************************************************************************

void
CloneNetworkRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: CloneNetworkRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 18, 2004
//
// ****************************************************************************

int
CloneNetworkRPC::GetID() const
{
    return id;
}

// ****************************************************************************
//  Method: TimeQueryRPC::SetQueryOverTimeAtts
//
//  Purpose:
//    This sets the query atts.
//
//  Arguments:
//    atts      the query atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 22, 2004 
//
// ****************************************************************************
 
void
CloneNetworkRPC::SetQueryOverTimeAtts(const QueryOverTimeAttributes *atts)
{
    queryAtts = *atts;
    Select(1, (void*)&queryAtts);
}


// ****************************************************************************
//  Method: CloneNetworkRPC::GetQueryOverTimeAtts
//
//  Purpose:
//    This returns the query atts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 22, 2004 
//
// ****************************************************************************
 
QueryOverTimeAttributes*
CloneNetworkRPC::GetQueryOverTimeAtts()
{
    return &queryAtts;
}
