/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                      avtLocateAndPickZoneQuery.C                          //
// ************************************************************************* //


#include <avtLocateAndPickZoneQuery.h>
#include <avtLocateCellQuery.h>
#include <avtParallel.h>
#include <avtQueryableSource.h>
#include <avtOriginatingSource.h>
#include <avtZonePickQuery.h>
#include <float.h>
#include <snprintf.h>
#include <DebugStream.h>
#include <PickVarInfo.h>
#include <QueryArgumentException.h>
#include <string>

// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery Constructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:09:18 PDT 2011
//    Added domain and zone.
//
// ****************************************************************************

avtLocateAndPickZoneQuery::avtLocateAndPickZoneQuery()
{
    lcq = new avtLocateCellQuery;
    zpq = new avtZonePickQuery;
    domain = 0;
    zone = 0;
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery Destructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

avtLocateAndPickZoneQuery::~avtLocateAndPickZoneQuery()
{
    if (lcq)
    {
        delete lcq;
        lcq = NULL;
    }
    if (zpq)
    {
        delete zpq;
        zpq = NULL;
    }
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::SetInputParams
//
//  Purpose: Allows this query to read any input parameters set by user.
//
//  Arguments:
//    params     The input parameters stored in a MapNode.
//
//  Programmer:  Kathleen Biagas 
//  Creation:    June 20, 2011
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 10 08:44:46 PST 2013
//    Add error checking on size of passed vectors. Use new MapNode methods 
//    for testing existence of numeric entries.
//
// ****************************************************************************

void
avtLocateAndPickZoneQuery::SetInputParams(const MapNode &params)
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

    doubleVector rp1, rp2;
    if (params.HasNumericVectorEntry("ray_start_point"))
    {
        params.GetEntry("ray_start_point")->ToDoubleVector(rp1); 
        if (rp1.size() != 3)
            EXCEPTION2(QueryArgumentException, "ray_start_point", 3);
        pickAtts.SetRayPoint1(rp1);
    }
    else
        EXCEPTION1(QueryArgumentException, "ray_start_point");

    if (params.HasEntry("ray_end_point"))
    {
        params.GetEntry("ray_end_point")->ToDoubleVector(rp2); 
        if (rp2.size() != 3)
            EXCEPTION2(QueryArgumentException, "ray_end_point", 3);
        pickAtts.SetRayPoint2(rp2);
    }
    else
        EXCEPTION1(QueryArgumentException, "ray_end_point");

    if (params.HasNumericEntry("domain"))
        domain = (params.GetEntry("domain")->ToInt());
    else
        EXCEPTION1(QueryArgumentException, "domain");

    if (params.HasNumericEntry("element"))
        zone = (params.GetEntry("element")->ToInt());
    else
        EXCEPTION1(QueryArgumentException, "element");
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::PerformQuery
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
//    Kathleen Biagas, Tue Jun 21 09:56:46 PDT 2011
//    RayPoints, vars, domain and element are retrieved now in SetInputParams.
//
// ****************************************************************************

void
avtLocateAndPickZoneQuery::PerformQuery(QueryAttributes *qa)
{
    // Preparation work
    avtDataRequest_p dataRequest = 
        GetInput()->GetOriginatingSource()->GetFullDataRequest();

    avtDataAttributes &inAtts = GetInput()->GetInfo().GetAttributes();
    avtDataValidity &inVal = GetInput()->GetInfo().GetValidity();
    pickAtts.SetActiveVariable(dataRequest->GetVariable());
    pickAtts.SetGhostType(inAtts.GetContainsGhostZones());

    pickAtts.SetTimeStep(qa->GetTimeStep());
    pickAtts.SetPickType(PickAttributes::Zone);

    // Do the locate part of the query
    lcq->SetInput(GetInput());
    lcq->SetPickAtts(&pickAtts);
    lcq->SetSILRestriction(querySILR); 
    lcq->SetTimeVarying(timeVarying);
    lcq->PerformQuery(qa);

    SetPickAtts(lcq->GetPickAtts());

    if (pickAtts.GetLocationSuccessful())
    {
        // Do the pick part of the query
        zpq->SetInput(GetInput()->GetQueryableSource()->GetOutput());
        zpq->SetPickAtts(&pickAtts);
        zpq->SetSILRestriction(querySILR); 
        zpq->SetSkippedLocate(false); 
        zpq->SetTimeVarying(true);
        zpq->SetNeedTransform(inVal.GetPointsWereTransformed());
        if (inAtts.HasInvTransform() && inAtts.GetCanUseInvTransform())
            zpq->SetInvTransform(inAtts.GetInvTransform());
        zpq->PerformQuery(qa);

        SetPickAtts(zpq->GetPickAtts());
    }

    // Post work
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
            vals = pickAtts.GetVarInfo(0).GetValues();
            qa->SetResultsValue(vals[vals.size()-1]);
        }
        else
        {
            char msg[120]; 
            SNPRINTF(msg, 120, "Could not retrieve information from domain "
                     " %d zone %d.", domain, zone);
            qa->SetResultsMessage(msg);
            qa->SetResultsValue(vals);
        }
    }
    pickAtts.PrepareForNewPick();
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::SetPickAtts
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
avtLocateAndPickZoneQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts = *pa;
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::SetInvTransform
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
avtLocateAndPickZoneQuery::SetInvTransform(const avtMatrix *m)
{
    zpq->SetInvTransform(m);
}


// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::SetNeedTransform
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
avtLocateAndPickZoneQuery::SetNeedTransform(const bool v)
{
    zpq->SetNeedTransform(v);
}

 
// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery::SetPickAttsForTimeQuery
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
avtLocateAndPickZoneQuery::SetPickAttsForTimeQuery(
    const PickAttributes *pa)
{
    pickAtts.SetMatSelected(pa->GetMatSelected() || 
        GetInput()->GetInfo().GetAttributes().MIROccurred());
    pickAtts.SetElementIsGlobal(pa->GetElementIsGlobal());
    pickAtts.SetTimePreserveCoord(pa->GetTimePreserveCoord());
    pickAtts.SetPlotBounds(pa->GetPlotBounds());
}


