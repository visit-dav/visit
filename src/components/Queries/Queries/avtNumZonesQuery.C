// ************************************************************************* //
//                            avtNumZonesQuery.C                             //
// ************************************************************************* //

#include <avtNumZonesQuery.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>


// ****************************************************************************
//  Method: avtNumZonesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNumZonesQuery::avtNumZonesQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtNumZonesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
avtNumZonesQuery::~avtNumZonesQuery() 
{
}


// ****************************************************************************
//  Method: avtNumZonesQuery::PerformQuery
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
avtNumZonesQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());
    SetTypedInput(dob);

    avtGhostType gt = GetInput()->GetInfo().GetAttributes().GetContainsGhostZones();

    avtDataset_p input = GetTypedInput();
    int totalZones [2] = {0, 0};
    char msg[1024];

    if (gt != AVT_HAS_GHOSTS)
    {
        totalZones[0] = avtDatasetExaminer::GetNumberOfZones(input);
        SumIntAcrossAllProcessors(totalZones[0]);
        if (OriginalData())
            sprintf(msg, "The original number of zones is %d.", totalZones[0]);
        else 
            sprintf(msg, "The actual number of zones is %d.", totalZones[0]);
        qA->SetResultsValue((double)totalZones[0]);
    }
    else 
    {
        avtDatasetExaminer::GetNumberOfZones(input, totalZones[0], totalZones[1]);
        int summedTZ[2];
        SumIntArrayAcrossAllProcessors(totalZones, summedTZ, 2);
        if (OriginalData())
        {
            sprintf(msg, "The original number of zones is %d.\n" 
              "The number of ghost zones is %d.", summedTZ[0], summedTZ[1]);
        }
        else 
        {
            sprintf(msg, "The actual number of zones is %d.\n" 
              "The number of ghost zones is %d.", summedTZ[0], summedTZ[1]);
        }
        double results[2] = {(double) summedTZ[0], (double) summedTZ[1]};
        qA->SetResultsValues(results, 2);
    }

    qA->SetResultsMessage(msg);
    UpdateProgress(1, 0);
}

