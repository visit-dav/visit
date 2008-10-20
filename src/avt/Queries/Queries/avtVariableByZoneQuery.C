/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                         avtVariableByZoneQuery.                           //
// ************************************************************************* //

#include <avtVariableByZoneQuery.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <float.h>
#include <snprintf.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtVariableByZoneQuery::avtVariableByZoneQuery
//
//  Purpose:
//      Construct an avtVariableByZoneQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtVariableByZoneQuery::avtVariableByZoneQuery()
{
}

// ****************************************************************************
//  Method: avtVariableByZoneQuery::~avtVariableByZoneQuery
//
//  Purpose:
//      Destruct an avtVariableByZoneQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtVariableByZoneQuery::~avtVariableByZoneQuery()
{
}

 
// ****************************************************************************
//  Method: avtVariableByZoneQuery::Preparation
//
//  Purpose:
//    Sets pickAtts based on queryAtts. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg.
//
// ****************************************************************************

void 
avtVariableByZoneQuery::Preparation(const avtDataAttributes &inAtts)
{
    avtDataRequest_p dataRequest = 
        GetInput()->GetOriginatingSource()->GetFullDataRequest();

    pickAtts.SetTimeStep(queryAtts.GetTimeStep());
    pickAtts.SetActiveVariable(dataRequest->GetVariable());
    pickAtts.SetDomain(queryAtts.GetDomain());
    pickAtts.SetElementNumber(queryAtts.GetElement());
    pickAtts.SetVariables(queryAtts.GetVariables());
    pickAtts.SetPickType(PickAttributes::DomainZone);

    avtPickByZoneQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtVariableByZoneQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 29, 2004 
//
//  Modifications:
//    Brad Whitlock, Tue Mar 13 11:26:59 PDT 2007
//    Updated due to code generation changes.
//
//    Kathleen Bonnell, Thu Nov 29 11:38:02 PST 2007
//    Ensure magnitude of vectors/tensors gets reported as the result, instead
//    of the first component.  Also ensure a failed query gets reported.
//
//    Brad Whitlock, Mon Oct 20 16:13:30 PDT 2008
//    Check to see if there are varInfo's in the pick attributes. There are
//    none in the case that the user accidentally performed the query on
//    a Mesh plot.
//
// ****************************************************************************

void
avtVariableByZoneQuery::PostExecute(void)
{
    avtPickQuery::PostExecute();
   
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
            SetResultMessage(msg.c_str());
            if(pickAtts.GetNumVarInfos() > 0)
            {
                vals = pickAtts.GetVarInfo(0).GetValues();
                SetResultValue(vals[vals.size()-1]);
            }
            else
                SetResultValues(vals);
        }
        else
        {
            char msg[120]; 
            SNPRINTF(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", queryAtts.GetDomain(), 
                     queryAtts.GetElement());
            SetResultMessage(msg);
            SetResultValues(vals);
        }
    }
    pickAtts.PrepareForNewPick();
}

// ****************************************************************************
//  Method: avtVariableByZoneQuery::GetTimeCurveSpecs
//
//  Purpose:
//    Override default TimeCurveSpecs 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 8, 2008 
//
//  Modifications:
//
// ****************************************************************************

const MapNode&
avtVariableByZoneQuery::GetTimeCurveSpecs()
{
    timeCurveSpecs["useVarForYAxis"] = true;
    return timeCurveSpecs;
}
