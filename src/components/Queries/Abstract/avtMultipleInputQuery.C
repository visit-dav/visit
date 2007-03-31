// ************************************************************************* //
//                        avtMultipleInputQuery.C                            //
// ************************************************************************* //

#include <avtMultipleInputQuery.h>


// ****************************************************************************
//  Method: avtMultipleInputQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   October 3, 2003 
//
// ****************************************************************************

avtMultipleInputQuery::avtMultipleInputQuery() : avtMultipleInputSink() 
{
    resValue = 0.;
}


// ****************************************************************************
//  Method: avtMultipleInputQuery::PerformQuery
//
//  Purpose:
//    Perform  the requested query. 
//
//  Programmer: Hank Childs 
//  Creation:   October 3, 2003 
//
// ****************************************************************************

void
avtMultipleInputQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;

    Execute();

    //
    // Retrieve the query results and set the message in the atts. 
    //
    queryAtts.SetResultsMessage(resMsg);
    queryAtts.SetResultsValue(resValue);
    UpdateProgress(1, 0);

    *qA = queryAtts;
}


