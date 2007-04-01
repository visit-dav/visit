// ************************************************************************* //
//                                avtTimeQuery.C                             //
// ************************************************************************* //

#include <avtTimeQuery.h>

#include <stdio.h>

#include <QueryAttributes.h>


// ****************************************************************************
//  Method: avtTimeQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTimeQuery::avtTimeQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTimeQuery::~avtTimeQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeQuery: PerformQuery
//
//  Purpose:
//      Gets the time.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

void
avtTimeQuery::PerformQuery(QueryAttributes *atts)
{
    double dtime = GetInput()->GetInfo().GetAttributes().GetTime();
    atts->SetResultsValue(dtime);
    std::string msg = GetResultMessage();
    atts->SetResultsMessage(msg);
}


// ****************************************************************************
//  Method: avtTimeQuery: GetResultMessage
//
//  Purpose:
//      Creates the result message.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

std::string
avtTimeQuery::GetResultMessage(void)
{
    double dtime = GetInput()->GetInfo().GetAttributes().GetTime();
    char msg[1024];
    sprintf(msg, "The time is %g.", dtime);
    std::string msg2 = msg;
    return msg2;
}


