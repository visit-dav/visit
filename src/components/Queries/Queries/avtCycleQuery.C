// ************************************************************************* //
//                               avtCycleQuery.C                             //
// ************************************************************************* //

#include <avtCycleQuery.h>

#include <QueryAttributes.h>


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


