// ************************************************************************* //
//                            avtNumZonesQuery.C                             //
// ************************************************************************* //

#include <avtNumZonesQuery.h>

#include <avtDatasetExaminer.h>
#include <snprintf.h>
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
//    Brad Whitlock, Mon Feb 23 12:11:02 PDT 2004
//    I made it use SNPRINTF to get it to build on Linux.
//    
//    Kathleen Bonnell, Fri Mar  5 15:36:23 PST 2004 
//    Rewrote code to work correctly in parallel when some processors have
//    not done any work and thus have a different value for ghost type. 
//    
//    Kathleen Bonnell, Tue Apr 20 12:49:46 PDT 2004 
//    Modified msg creation so that it works properly on all platforms. 
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Kathleen Bonnell, Tue Jul  6 14:32:06 PDT 2004 
//    Removed MPI calls, use SumIntArrayAcrossAllProcessors from avtParallel. 
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

    int usedDomains = 
        GetInput()->GetInfo().GetValidity().GetHasEverOwnedAnyDomain() ? 1 : 0;

    avtGhostType gt = GetInput()->GetInfo().GetAttributes().GetContainsGhostZones();
    int totalZones [2] = {0, 0};
    char msg[200];
    if (usedDomains)
    {
        avtDataset_p input = GetTypedInput();

        if (gt != AVT_HAS_GHOSTS)
        {
            totalZones[0] = avtDatasetExaminer::GetNumberOfZones(input);
        }
        else 
        {
            avtDatasetExaminer::GetNumberOfZones(input, totalZones[0], totalZones[1]);
        }
    }

    int tz[2] = {0, 0};
    SumIntArrayAcrossAllProcessors(totalZones, tz, 2);

    if (OriginalData())
        SNPRINTF(msg, 200, "The original number of zones is %d.", tz[0]);
    else 
        SNPRINTF(msg, 200, "The actual number of zones is %d.", tz[0]);
    
    if (gt != AVT_HAS_GHOSTS)
    {
        qA->SetResultsValue((double)tz[0]);
        qA->SetResultsMessage(msg);
    }
    else
    {
        char msg2[200];
        SNPRINTF(msg2, 200, "%s\nThe number of ghost zones is %d.", msg, tz[1]);
        double results[2] = {(double) tz[0], (double) tz[1]};
        qA->SetResultsValues(results, 2);
        qA->SetResultsMessage(msg2);
    }
    UpdateProgress(1, 0);
}

