#include "QueryRPC.h" 


// ****************************************************************************
//  Method: QueryRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
// ****************************************************************************

QueryRPC::QueryRPC() : NonBlockingRPC("ia", &returnAtts)
{
  // nothing here;
}


// ****************************************************************************
//  Method: QueryRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
// ****************************************************************************

void
QueryRPC::operator()(const int netId, const QueryAttributes *atts)
{
    SetNetworkId(netId);
    SetQueryAtts(atts);
    Execute();
}


// ****************************************************************************
//  Method: QueryRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
// ****************************************************************************

void
QueryRPC::SelectAll()
{
    Select(0, (void*)&networkId);
    Select(1, (void*)&queryAtts);
}

 
// ****************************************************************************
//  Method: QueryRPC::SetQueryAtts
//
//  Purpose:
//    This sets the query atts.
//
//  Arguments:
//    atts      the query atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 20, 2002 
//
// ****************************************************************************
 
void
QueryRPC::SetQueryAtts(const QueryAttributes *atts)
{
    queryAtts = *atts;
    Select(1, (void*)&queryAtts);
}


// ****************************************************************************
//  Method: QueryRPC::GetQueryAtts
//
//  Purpose:
//    This returns the query atts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 20, 2002
//
// ****************************************************************************
 
QueryAttributes*
QueryRPC::GetQueryAtts()
{
    return &queryAtts;
}
