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
//                           avtTrajectoryByZone.                            //
// ************************************************************************* //

#include <avtTrajectoryByZone.h>
#include <avtParallel.h>
#include <float.h>
#include <snprintf.h>
#include <PickVarInfo.h>


using std::string;



// ****************************************************************************
//  Method: avtTrajectoryByZone::avtTrajectoryByZone
//
//  Purpose:
//      Construct an avtTrajectoryByZone object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByZone::avtTrajectoryByZone()
{
}

// ****************************************************************************
//  Method: avtTrajectoryByZone::~avtTrajectoryByZone
//
//  Purpose:
//      Destruct an avtTrajectoryByZone object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005 
//
//  Modifications:
//
// ****************************************************************************

avtTrajectoryByZone::~avtTrajectoryByZone()
{
}

 
// ****************************************************************************
//  Method: avtTrajectoryByZone::Preparation
//
//  Purpose:
//    Sets pickAtts based on queryAtts. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 8, 2005
//
//  Modifications:
//
// ****************************************************************************

void 
avtTrajectoryByZone::Preparation(const avtDataAttributes &inAtts)
{
    if (inAtts.ValidVariable(queryAtts.GetVariables()[0].c_str()))
        queryAtts.SetXUnits(inAtts.GetVariableUnits(queryAtts.GetVariables()[0].c_str()));
    if (inAtts.ValidVariable(queryAtts.GetVariables()[1].c_str()))
        queryAtts.SetYUnits(inAtts.GetVariableUnits(queryAtts.GetVariables()[1].c_str()));
 
    avtVariableByZoneQuery::Preparation(inAtts);
}


// ****************************************************************************
//  Method: avtTrajectoryByZone::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 8, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue Mar 13 11:27:42 PDT 2007
//    Updated due to code generation changes.
//
// ****************************************************************************

void
avtTrajectoryByZone::PostExecute(void)
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
            doubleVector res;
            res.push_back(pickAtts.GetVarInfo(0).GetValues()[0]);
            res.push_back(pickAtts.GetVarInfo(1).GetValues()[0]);
            SetResultValues(res);

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

