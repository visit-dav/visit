// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLocateAndPickNodeQuery.C                          //
// ************************************************************************* //


#include <avtLocateAndPickNodeQuery.h>
#include <avtLocateNodeQuery.h>
#include <avtParallel.h>
#include <avtQueryableSource.h>
#include <avtOriginatingSource.h>
#include <avtNodePickQuery.h>
#include <float.h>
#include <DebugStream.h>
#include <PickVarInfo.h>
#include <QueryArgumentException.h>
#include <string>

// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery Constructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:43:25 PDT 2011
//    Added domain, node.
//
// ****************************************************************************

avtLocateAndPickNodeQuery::avtLocateAndPickNodeQuery()
{
    lnq = new avtLocateNodeQuery;
    npq = new avtNodePickQuery;
    domain = 0;
    node = 0;
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery Destructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

avtLocateAndPickNodeQuery::~avtLocateAndPickNodeQuery()
{
    if (lnq)
    {
        delete lnq;
        lnq = NULL;
    }
    if (npq)
    {
        delete npq;
        npq = NULL;
    }
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::SetInputParams
//
//  Purpose:
//    Allows this query to read input parameters set by user.
//
//  Arguments:
//    params:  MapNode containing input.
//
//  Programmer:  Kathleen Biagas 
//  Creation:    June 20, 2011 
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 10 08:37:48 PST 2013
//    Add error checking on size of passed vectors. Use new MapNode methods
//    for testing of numeric entries.
//
// ****************************************************************************

void
avtLocateAndPickNodeQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("vars"))
    {
        const stringVector &v = params.GetEntry("vars")->AsStringVector();
        if (v.empty())
            EXCEPTION2(QueryArgumentException, "vars", 1);
        timeCurveSpecs["nResultsToStore"] = (int)v.size();
        pickAtts.SetVariables(v);
    }
    else
        EXCEPTION1(QueryArgumentException, "vars");

    if (params.HasNumericVectorEntry("ray_start_point"))
    {
        doubleVector v;
        params.GetEntry("ray_start_point")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "ray_start_point", 3);
        pickAtts.SetRayPoint1(v);
    }
    else
        EXCEPTION1(QueryArgumentException, "ray_start_point");

    if (params.HasNumericVectorEntry("ray_end_point"))
    {
        doubleVector v;
        params.GetEntry("ray_end_point")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "ray_end_point", 3);
        pickAtts.SetRayPoint2(v);
    }
    else
        EXCEPTION1(QueryArgumentException, "ray_end_point");

    if (params.HasNumericEntry("domain"))
        domain = params.GetEntry("domain")->ToInt();
    else
        EXCEPTION1(QueryArgumentException, "domain");

    if (params.HasNumericEntry("element"))
        node = params.GetEntry("element")->ToInt();
    else
        EXCEPTION1(QueryArgumentException, "element");
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::PerformQuery
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov 29 11:38:02 PST 2007
//    Ensure magnitude of vectors/tensors gets reported as the result, instead
//    of the first component.  Also ensure a failed query gets reported.
//
//    Kathleen Bonnell, Thu Feb 17 09:57:58 PST 2011
//    Allow multiple outputs / results.
//
//    Kathleen Biagas, Tue Jun 21 10:51:24 PDT 2011
//    Domain, Node, RayPoints and variables retrieved during SetInputParams.
//
// ****************************************************************************

void
avtLocateAndPickNodeQuery::PerformQuery(QueryAttributes *qa)
{
    // Preparation work
    avtDataRequest_p dataRequest = 
        GetInput()->GetOriginatingSource()->GetFullDataRequest();

    avtDataAttributes &inAtts = GetInput()->GetInfo().GetAttributes();
    avtDataValidity &inVal = GetInput()->GetInfo().GetValidity();

    pickAtts.SetActiveVariable(dataRequest->GetVariable());
    pickAtts.SetGhostType(inAtts.GetContainsGhostZones());
    pickAtts.SetTimeStep(qa->GetTimeStep());
    pickAtts.SetPickType(PickAttributes::Node);

    // Do the locate part of the query
    lnq->SetInput(GetInput());
    lnq->SetPickAtts(&pickAtts);
    lnq->SetSILRestriction(querySILR); 
    lnq->SetTimeVarying(true);
    lnq->PerformQuery(qa);

    SetPickAtts(lnq->GetPickAtts());

    if (pickAtts.GetLocationSuccessful())
    {
        // Do the pick part of the query
        npq->SetInput(GetInput()->GetQueryableSource()->GetOutput());
        npq->SetPickAtts(&pickAtts);
        npq->SetSILRestriction(querySILR); 
        npq->SetSkippedLocate(false); 
        npq->SetTimeVarying(true);
        npq->SetNeedTransform(inVal.GetPointsWereTransformed());
        if (inAtts.HasInvTransform() && inAtts.GetCanUseInvTransform())
            npq->SetInvTransform(inAtts.GetInvTransform());
        npq->PerformQuery(qa);

        SetPickAtts(npq->GetPickAtts());
    }

    if (PAR_Rank() == 0) 
    {
        doubleVector vals;
        if (pickAtts.GetFulfilled())
        {
            // Special indication that the pick point should not be displayed.
            double cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            std::string msg;
            pickAtts.SetCellPoint(cp);
            pickAtts.CreateOutputString(msg);
            qa->SetResultsMessage(msg);
            if ( pickAtts.GetNumVarInfos() == 1)
            {
                vals = pickAtts.GetVarInfo(0).GetValues();
                qa->SetResultsValue(vals[vals.size()-1]);
            }
            else
            {
                for (int i = 0; i < pickAtts.GetNumVarInfos(); ++i)
                {
                    vals.push_back(pickAtts.GetVarInfo(i).GetValues()[0]);
                }
                qa->SetResultsValues(&vals[0], (int)vals.size());
            }
        }
        else
        {
            char msg[120]; 
            snprintf(msg, 120, "Could not retrieve information from domain "
                     " %d node %d.", domain, node);
            qa->SetResultsMessage(msg);
            qa->SetResultsValue(vals);
        }
    }
    pickAtts.PrepareForNewPick();
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::SetPickAtts
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

void
avtLocateAndPickNodeQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts = *pa;
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::SetInvTransform
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

void
avtLocateAndPickNodeQuery::SetInvTransform(const avtMatrix *m)
{
    npq->SetInvTransform(m);
}


// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::SetNeedTransform
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

void
avtLocateAndPickNodeQuery::SetNeedTransform(const bool v)
{
    npq->SetNeedTransform(v);
}

 
// ****************************************************************************
//  Method: avtLocateAndPickNodeQuery::SetPickAttsForTimeQuery
//
//  Purpose:
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 9, 2007
//
//  Modifications:
//
// ****************************************************************************

void                      
avtLocateAndPickNodeQuery::SetPickAttsForTimeQuery(
    const PickAttributes *pa)
{
    pickAtts.SetMatSelected(pa->GetMatSelected() || 
        GetInput()->GetInfo().GetAttributes().MIROccurred());
    pickAtts.SetElementIsGlobal(pa->GetElementIsGlobal());
    pickAtts.SetTimePreserveCoord(pa->GetTimePreserveCoord());
    pickAtts.SetPlotBounds(pa->GetPlotBounds());
}
