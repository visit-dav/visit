// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Method: avtNumZonesQuery::GetDefaultInputParams
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 26, 2011
//
//  Modifications:
//    Kathleen Biagas, Thu Jun  8 12:36:29 PDT 2017
//    Change the default to 1, for consistency with other queries.
//
// ****************************************************************************

void
avtNumZonesQuery::GetDefaultInputParams(MapNode &params)
{
    params["use_actual_data"] = 1;
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
//    I made it use snprintf to get it to build on Linux.
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
//    Hank Childs, Sat Nov 21 13:10:43 PST 2009
//    Change counter to long long.
//
//    Kathleen Biagas, Wed Feb 12 09:07:34 PST 2014
//    Add xml results.
//
//    Kathleen Biagas, Thu Sep 11 09:13:45 PDT 2014
//    Added flag to count original zones only (may be needed for arbpoly
//    data that was split by the DB reader.
//
//    Kathleen Biagas, Wed Nov 18 2020
//    Replace VISIT_LONG_LONG with long long.
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

    bool countOriginalOnly = OriginalData() &&
            !GetInput()->GetInfo().GetValidity().GetZonesPreserved() &&
             GetInput()->GetInfo().GetAttributes().GetContainsOriginalCells();

    avtGhostType gt = GetInput()->GetInfo().GetAttributes().GetContainsGhostZones();
    long long totalZones [2] = {0, 0};
    char msg[200];
    if (usedDomains)
    {
        avtDataset_p input = GetTypedInput();

        if (gt != AVT_HAS_GHOSTS)
        {
            totalZones[0] = avtDatasetExaminer::GetNumberOfZones(input, countOriginalOnly);
        }
        else 
        {
            avtDatasetExaminer::GetNumberOfZones(input, totalZones[0], totalZones[1],
                                                 countOriginalOnly);
        }
    }

    long long tz[2] = {0, 0};
    SumLongLongArrayAcrossAllProcessors(totalZones, tz, 2);

    MapNode result_node;
    result_node["num_zones"] = (int)tz[0];
 
    if (OriginalData())
        snprintf(msg, 200, "The original number of zones is %lld.", tz[0]);
    else 
        snprintf(msg, 200, "The actual number of zones is %lld.", tz[0]);
    
    if (gt != AVT_HAS_GHOSTS)
    {
        qA->SetResultsValue((double)tz[0]);
        qA->SetResultsMessage(msg);
    }
    else
    {
        char msg2[200];
        snprintf(msg2, 200, "%s\nThe number of ghost zones is %lld.", msg, tz[1]);
        double results[2] = {(double) tz[0], (double) tz[1]};
        qA->SetResultsValues(results, 2);
        qA->SetResultsMessage(msg2);
        result_node["num_ghost_zones"] = (int)tz[1];
    }
    qA->SetXmlResult(result_node.ToXML());

    UpdateProgress(1, 0);
}

