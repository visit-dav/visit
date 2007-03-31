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
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:25:55 PDT 2003
//    Added '*' to 'i*' since we now have a vector of ints.
//
// ****************************************************************************

QueryRPC::QueryRPC() : NonBlockingRPC("i*a", &returnAtts)
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
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:25:55 PDT 2003
//    Renamed networkId to networkIds.
//
// ****************************************************************************

void
QueryRPC::operator()(const std::vector<int> &netIds,
                     const QueryAttributes *atts)
{
    SetNetworkIds(netIds);
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
    Select(0, (void*)&networkIds);
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
