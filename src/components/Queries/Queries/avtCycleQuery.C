// ************************************************************************* //
//                               avtCycleQuery.C                             //
// ************************************************************************* //

#include <avtCycleQuery.h>

#include <stdio.h>

#include <QueryAttributes.h>


// ****************************************************************************
//  Method: avtCycleQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCycleQuery::avtCycleQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtCycleQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCycleQuery::~avtCycleQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtCycleQuery: PerformQuery
//
//  Purpose:
//      Gets the cycle.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

void
avtCycleQuery::PerformQuery(QueryAttributes *atts)
{
    int cycle = GetInput()->GetInfo().GetAttributes().GetCycle();
    atts->SetResultsValue((double) cycle);
    std::string msg = GetResultMessage();
    atts->SetResultsMessage(msg);
}


// ****************************************************************************
//  Method: avtCycleQuery: GetResultMessage
//
//  Purpose:
//      Creates the result message.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

std::string
avtCycleQuery::GetResultMessage(void)
{
    int cycle = GetInput()->GetInfo().GetAttributes().GetCycle();
    char msg[1024];
    sprintf(msg, "The cycle is %d.", cycle);
    std::string msg2 = msg;
    return msg2;
}


