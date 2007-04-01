// ************************************************************************* //
//                            avtNumNodesQuery.C                             //
// ************************************************************************* //

#include <avtNumNodesQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>


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
    char msg[1024];

    totalNodes = avtDatasetExaminer::GetNumberOfNodes(input);
    SumIntAcrossAllProcessors(totalNodes);
    if (OriginalData())
        sprintf(msg, "The original number of nodes is %d.", totalNodes);
    else 
        sprintf(msg, "The actual number of nodes is %d.", totalNodes);

    qA->SetResultsMessage(msg);
    qA->SetResultsValue((double)totalNodes);

    UpdateProgress(1, 0);
}

