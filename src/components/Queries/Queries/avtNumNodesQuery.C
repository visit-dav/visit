// ************************************************************************* //
//                            avtNumNodesQuery.C                             //
// ************************************************************************* //

#include <avtNumNodesQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>
#include <snprintf.h>

// ****************************************************************************
//  Method: avtNumNodesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNumNodesQuery::avtNumNodesQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtNumNodesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
avtNumNodesQuery::~avtNumNodesQuery() 
{
}


// ****************************************************************************
//  Method: avtNumNodesQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Brad Whitlock, Mon Feb 23 12:11:02 PDT 2004
//    I made it use SNPRINTF to get it to build on Linux.
//
// ****************************************************************************

void
avtNumNodesQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());

    SetTypedInput(dob);

    avtDataset_p input = GetTypedInput();
    int totalNodes = 0;
    char msg[200];

    totalNodes = avtDatasetExaminer::GetNumberOfNodes(input);
    SumIntAcrossAllProcessors(totalNodes);
    if (OriginalData())
        SNPRINTF(msg, 200, "The original number of nodes is %d.", totalNodes);
    else 
        SNPRINTF(msg, 200, "The actual number of nodes is %d.", totalNodes);

    qA->SetResultsMessage(msg);
    qA->SetResultsValue((double)totalNodes);

    UpdateProgress(1, 0);
}

