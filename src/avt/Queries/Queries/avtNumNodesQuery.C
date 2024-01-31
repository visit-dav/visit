// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Method: avtNumNodesQuery::GetDefaultInputParams
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
avtNumNodesQuery::GetDefaultInputParams(MapNode &params)
{
    params["use_actual_data"] = 1;
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
//    I made it use snprintf to get it to build on Linux.
//
//    Kathleen Bonnell, Tue Jul 29 10:01:29 PDT 2008
//    Ensure ghost nodes aren't counted in the 'real' count by calling the
//    appropriate avtDatasetExaminer method in the presence of ghost zones.
//
//    Hank Childs, Sat Nov 21 13:25:42 PST 2009
//    Add long long support.
//
//    Kathleen Biagas, Wed Feb 12 08:17:21 PST 2014
//    Add xml results.
//
//    Kathleen Biagas, Thu Sep 11 09:13:45 PDT 2014
//    Added flag to count original nodes only (may be needed for arbpoly
//    data that was split by the DB reader.
//
//    Kathleen Biagas, Wed Nov 18 2020
//    Replace VISIT_LONG_LONG with long long.
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

    int usedDomains = 
        GetInput()->GetInfo().GetValidity().GetHasEverOwnedAnyDomain() ? 1 : 0;

    avtGhostType gzt = 
        GetInput()->GetInfo().GetAttributes().GetContainsGhostZones();

    bool countOriginalOnly = OriginalData() &&
            !GetInput()->GetInfo().GetValidity().GetZonesPreserved() &&
             GetInput()->GetInfo().GetAttributes().GetContainsOriginalNodes();

    long long totalNodes[2] = {0, 0};
    long long tn[2] = {0, 0};
    char msg[200];
    if (usedDomains)
    {
        avtDataset_p input = GetTypedInput();
        if (gzt != AVT_HAS_GHOSTS)
        {
            totalNodes[0] = avtDatasetExaminer::GetNumberOfNodes(input,
                                                 countOriginalOnly);
        }
        else 
        {
            avtDatasetExaminer::GetNumberOfNodes(input, totalNodes[0], 
                                                 totalNodes[1],
                                                 countOriginalOnly);
        }
    }

    SumLongLongArrayAcrossAllProcessors(totalNodes, tn, 2);

    MapNode result_node;
    result_node["num_nodes"] = (int)tn[0];
    if (OriginalData())
        snprintf(msg, 200, "The original number of nodes is %lld.", tn[0]);
    else 
        snprintf(msg, 200, "The actual number of nodes is %lld.", tn[0]);


    if (gzt != AVT_HAS_GHOSTS)
    {
        qA->SetResultsValue((double)tn[0]);
        qA->SetResultsMessage(msg);
    }
    else
    {
        char msg2[200];
        snprintf(msg2, 200, "%s\nThe number of ghost nodes is %lld.", msg, tn[1]);
        double results[2] = {(double) tn[0], (double) tn[1]};
        qA->SetResultsValues(results, 2);
        qA->SetResultsMessage(msg2);
        result_node["num_ghost_nodes"] = (int)tn[1];
    }
    qA->SetXmlResult(result_node.ToXML());
    UpdateProgress(1, 0);
}

