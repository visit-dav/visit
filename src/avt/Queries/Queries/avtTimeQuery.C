// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtTimeQuery.C                             //
// ************************************************************************* //

#include <avtTimeQuery.h>

#include <snprintf.h>

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
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:57:12 PDT 2007
//    Keep copy of passed QueryAttributes in queryAtts
//
// ****************************************************************************

void
avtTimeQuery::PerformQuery(QueryAttributes *atts)
{
    double dtime = GetInput()->GetInfo().GetAttributes().GetTime();
    atts->SetResultsValue(dtime);
    queryAtts = *atts;
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
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

std::string
avtTimeQuery::GetResultMessage()
{
    double dtime = GetInput()->GetInfo().GetAttributes().GetTime();
    char msg[1024];
    
    std::string format = "The time is " + queryAtts.GetFloatFormat() + ".";
    SNPRINTF(msg,1024,format.c_str(), dtime);
    std::string msg2 = msg;
    return msg2;
}


