/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                     avtExecuteThenTimeLoopFilter.C                        //
// ************************************************************************* //

#include <avtExecuteThenTimeLoopFilter.h>

#include <avtCallback.h>
#include <avtDataAttributes.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtExtents.h>
#include <avtNamedSelectionFilter.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>



// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2008
//
//  Modifications:
//    Hank Childs, Thu Jan 27 09:14:35 PST 2005
//    Changed inital values of time-loop vars to -1, so they could be set
//    individually by user.
//
// ****************************************************************************

avtExecuteThenTimeLoopFilter::avtExecuteThenTimeLoopFilter()
{
    startTime = -1;
    endTime = -1;
    stride = -1; 
    nFrames = 0;
    actualEnd = 0;
}


// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2008 
//
// ****************************************************************************

avtExecuteThenTimeLoopFilter::~avtExecuteThenTimeLoopFilter()
{
}


// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter::FinalizeTimeLoop
//
//  Purpose:
//      Performs correctness checking.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2008
//
// ****************************************************************************

void
avtExecuteThenTimeLoopFilter::FinalizeTimeLoop()
{
    int numStates = GetInput()->GetInfo().GetAttributes().GetNumStates();

    if (startTime < 0)
    {
        startTime = 0;
    }
    if (endTime < 0)
    {
        endTime = numStates - 1;
    }
    if (stride < 0)
    {
        stride = 1;
    }

    if (startTime > endTime)
    {
        std::string msg("Start time must be smaller than or equal to the end time for " );
        msg += GetType();
        msg += ".\n";
        EXCEPTION1(ImproperUseException, msg);
    }

    nFrames = (int) ceil((((float)endTime-startTime))/(float)stride) + 1; 

    // ARS - This check will always pass because of the additional
    // frame added to nFrames above and startTime must be greater than
    // or equal to endTime. Thus it is commented out.

//     if (nFrames < 1)
//     {
//         std::string msg(GetType());
//         msg = msg +  " requires at least 1 frame, please correct start " + 
//                "and end times and try again.";
//         EXCEPTION1(ImproperUseException, msg);
//    }

    if (endTime >= numStates)
    {
        std::string msg(GetType());
        msg += ":  Clamping end time to number of available timesteps.";
        avtCallback::IssueWarning(msg.c_str());
    }

    //
    // Ensure that the specified endTime is included,
    // regardless of the stride.
    //
    actualEnd = startTime + nFrames * stride;
    if (actualEnd < endTime)
        actualEnd = endTime + stride;
}


// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter::Execute
//
//  Purpose: 
//    Defines what it means for this filter to "Execute".  This means allowing
//    its derived types to inspect the principal data, and then iterating
//    over time slices.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 19:20:14 PST 2009
//    Use the contract from the original execution.
//
//    Hank Childs, Tue Jan  5 13:14:37 PST 2010
//    Merge in the extents from each time slice.
//
//    Hank Childs, Wed Jan  6 16:13:00 PST 2010
//    Only merge the extents if they have the same dimension.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Brad Whitlock, Thu Sep  1 10:50:43 PDT 2011
//    Add in named selection filter.
//
// ****************************************************************************

void
avtExecuteThenTimeLoopFilter::Execute(void)
{
    if (*origContract == NULL)
    {
        // You're derived type reimplemented ModifyContract and you need to
        // make it call SetContract.
        EXCEPTION0(ImproperUseException);
    }

    FinalizeTimeLoop();
    int i;

    InspectPrincipalData();

    for (i = startTime; i < actualEnd; i+= stride)
    {
        avtOriginatingSource *src = GetOriginatingSource();
        avtExpressionEvaluatorFilter eef;
        eef.SetInput(src->GetOutput());

        avtContract_p contract = new avtContract(origContract);
        int currentTime = (i < endTime ? i : endTime);

        debug5 << "Execute-then-time-loop-filter updating with time slice #" 
               << currentTime << endl;
        contract->GetDataRequest()->SetTimestep(currentTime);

        avtDataset *ds = NULL;
        avtNamedSelectionFilter nsf;
        if(contract->GetDataRequest()->GetSelectionName().empty())
        {
            eef.Update(contract);
            ds = *(eef.GetTypedOutput());
        }
        else
        {
            nsf.SetInput(eef.GetOutput());
            nsf.SetSelectionName(contract->GetDataRequest()->GetSelectionName());
            nsf.Update(contract);
            ds = *(nsf.GetTypedOutput());
        }

        // Friend status plus reference points leads to some extra
        // contortions here.
        avtDataTree_p tree = ds->GetDataTree();
        Iterate(currentTime, tree);

        // Merge the extents
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
        avtDataAttributes &inAtts = ds->GetInfo().GetAttributes();
        if (outAtts.GetSpatialDimension() == inAtts.GetSpatialDimension())
        {
            outAtts.GetOriginalSpatialExtents()->Merge(*(inAtts.GetOriginalSpatialExtents()));
            outAtts.GetThisProcsOriginalSpatialExtents()->Merge(*(inAtts.GetThisProcsOriginalSpatialExtents()));
            outAtts.GetDesiredSpatialExtents()->Merge(*(inAtts.GetDesiredSpatialExtents()));
            outAtts.GetActualSpatialExtents()->Merge(*(inAtts.GetActualSpatialExtents()));
            outAtts.GetThisProcsActualSpatialExtents()->Merge(*(inAtts.GetThisProcsActualSpatialExtents()));
        }
    
        for (int j = 0 ; j < outAtts.GetNumberOfVariables() ; j++)
        {
            const char *vname = outAtts.GetVariableName(j).c_str();
            outAtts.GetOriginalDataExtents(vname)->Merge(*(inAtts.GetOriginalDataExtents(vname)));
            outAtts.GetThisProcsOriginalDataExtents(vname)->Merge(*(inAtts.GetThisProcsOriginalDataExtents(vname)));
            outAtts.GetDesiredDataExtents(vname)->Merge(*(inAtts.GetDesiredDataExtents(vname)));
            outAtts.GetActualDataExtents(vname)->Merge(*(inAtts.GetActualDataExtents(vname)));
            outAtts.GetThisProcsActualDataExtents(vname)->Merge(*(inAtts.GetThisProcsActualDataExtents(vname)));
        }

        avtCallback::ResetTimeout(5*60);
    } 

    Finalize();
}
