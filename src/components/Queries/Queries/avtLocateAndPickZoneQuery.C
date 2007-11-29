/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
#include <avtTerminatingSource.h>
#include <avtZonePickQuery.h>
#include <float.h>
#include <snprintf.h>
#include <DebugStream.h>
#include <PickVarInfo.h>

// ****************************************************************************
//  Method: avtLocateAndPickZoneQuery Constructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//
// ****************************************************************************

avtLocateAndPickZoneQuery::avtLocateAndPickZoneQuery()
{
    lcq = new avtLocateCellQuery;
    zpq = new avtZonePickQuery;
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
// ****************************************************************************

void
avtLocateAndPickZoneQuery::PerformQuery(QueryAttributes *qa)
{
    // Preparation work
    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();

    avtDataAttributes &inAtts = GetInput()->GetInfo().GetAttributes();
    avtDataValidity &inVal = GetInput()->GetInfo().GetValidity();
    pickAtts.SetActiveVariable(dspec->GetVariable());
    pickAtts.SetGhostType(inAtts.GetContainsGhostZones());

    pickAtts.SetTimeStep(qa->GetTimeStep());
    pickAtts.SetRayPoint1(qa->GetDarg1());
    pickAtts.SetRayPoint2(qa->GetDarg2());
    pickAtts.SetVariables(qa->GetVariables());
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
            string msg;
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
                     " %d element %d.", queryAtts.GetDomain(), 
                     queryAtts.GetElement());
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
