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
//                          avtVariableByNodeQuery.C                         //
// ************************************************************************* //

#include <avtVariableByNodeQuery.h>
#include <avtTerminatingSource.h>
#include <avtParallel.h>
#include <float.h>
#include <snprintf.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtVariableByNodeQuery::avtVariableByNodeQuery
//
//  Purpose:
//      Construct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtVariableByNodeQuery::avtVariableByNodeQuery()
{
}

// ****************************************************************************
//  Method: avtVariableByNodeQuery::~avtVariableByNodeQuery
//
//  Purpose:
//      Destruct an avtVariableByNodeQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtVariableByNodeQuery::~avtVariableByNodeQuery()
{
}

 
// ****************************************************************************
//  Method: avtVariableByNodeQuery::Preparation
//
//  Purpose:
//    Sets pickAtts information from queryAtts. 
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
avtVariableByNodeQuery::Preparation(const avtDataAttributes &inAtts)
{
    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();

    pickAtts.SetTimeStep(queryAtts.GetTimeStep());
    pickAtts.SetActiveVariable(dspec->GetVariable());
    pickAtts.SetDomain(queryAtts.GetDomain());
    pickAtts.SetElementNumber(queryAtts.GetElement());
    pickAtts.SetVariables(queryAtts.GetVariables());
    pickAtts.SetPickType(PickAttributes::DomainNode);

    avtPickByNodeQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtVariableByNodeQuery::PostExecute
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
// ****************************************************************************

void
avtVariableByNodeQuery::PostExecute(void)
{
    avtPickQuery::PostExecute();
   
    if (PAR_Rank() == 0) 
    {
        if (pickAtts.GetFulfilled())
        {
            // Special indication that the pick point should not be displayed.
            double cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            string msg;
            pickAtts.SetCellPoint(cp);
            pickAtts.CreateOutputString(msg);
            SetResultMessage(msg.c_str());
            SetResultValues(pickAtts.GetVarInfo(0).GetValues());
        }
        else
        {
            char msg[120]; 
            SNPRINTF(msg, 120, "Could not retrieve information from domain "
                     " %d element %d.", queryAtts.GetDomain(), queryAtts.GetElement());
            SetResultMessage(msg);
        }
    }
    pickAtts.PrepareForNewPick();
}

