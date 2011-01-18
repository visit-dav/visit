/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                          avtTimeLoopFilter.C                              //
// ************************************************************************* //

#include <avtTimeLoopFilter.h>

#include <avtCallback.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <math.h>


// ****************************************************************************
//  Method: avtTimeLoopFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005
//    Changed inital values of time-loop vars to -1, so they could be set
//    individually by user.
//
//    Hank Childs, Wed Dec 15 14:30:42 PST 2010
//    Initialize parallelizingOverTime.
//
// ****************************************************************************

avtTimeLoopFilter::avtTimeLoopFilter()
{
    startTime = -1;
    endTime = -1;
    stride = -1; 
    nFrames = 0;
    actualEnd = 0;
    parallelizingOverTime = false;
}


// ****************************************************************************
//  Method: avtTimeLoopFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 29, 2004 
//
// ****************************************************************************

avtTimeLoopFilter::~avtTimeLoopFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeLoopFilter::Update
//
//  Purpose: 
//    Loops through specified timesteps:  retrieves correct SILRestriction,
//    creates appropriate avtDataRequest and avtContract
//    for each timestep and calls avtFilter::Update to initiate a new 
//    pipeline execution for each timestep. 
//
//  Arguments:
//      spec    The pipeline specification.
//
//  Returns:    A Boolean indicating if anything upstream was modified.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005
//    Added call to FinalizeTimeLoop.
//
//    Hank Childs, Fri Mar  4 09:41:53 PST 2005
//    Don't do dynamic load balancing with time queries.
//
//    Kathleen Bonnell, Tue Mar 15 17:41:26 PST 2005
//    Update Queryable source at end, to ensure the pipeline is in the same 
//    state as when we began.
//
//    Hank Childs, Tue Sep  5 14:07:59 PDT 2006
//    Reset the timeout through a callback, in case this takes a long time.
//
//    Hank Childs, Wed Feb  7 13:18:28 PST 2007
//    Coordinate with the terminating source (which issues progress
//    updates) to get better progress.
//
//    Hank Childs, Thu Jan 24 12:56:48 PST 2008
//    Add debug statement.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Wed Dec 15 14:30:42 PST 2010
//    Add support for parallelizing over time.
//
// ****************************************************************************

bool
avtTimeLoopFilter::Update(avtContract_p spec)
{
    int i;
    bool modified = false;
    avtDataRequest_p orig_DS = spec->GetDataRequest();
    avtSILRestriction_p orig_SILR = orig_DS->GetRestriction();

    parallelizingOverTime = CanDoTimeParallelization();

    FinalizeTimeLoop();

    //
    // This will tell the terminating source how many executions we are doing.
    // That in turn will allow the progress to work correctly.
    //
    int numIters = (actualEnd-startTime)/stride+1;
    avtOriginatingSource *src = GetOriginatingSource();
    src->SetNumberOfExecutions(numIters);

    int curIter = 0;
    for (i = startTime; i < actualEnd; i+= stride)
    {
        bool shouldDoThisTimeSlice = true;
        if (parallelizingOverTime)
            if ((curIter % PAR_Size()) != PAR_Rank())
                shouldDoThisTimeSlice = false;
        curIter++;
        if (!shouldDoThisTimeSlice)
            continue;
             
        if (i < endTime)
           currentTime = i; 
        else 
           currentTime = endTime; 
        debug4 << "Time loop filter updating with time slice #" 
               << currentTime << endl;

        avtSIL *sil = GetInput()->GetOriginatingSource()->GetSIL(currentTime);
        if (sil == NULL)
        {
            debug4 << "Could not read the SIL at state " << currentTime << endl;
            currentSILR = orig_SILR;
        }
        else 
        {
            currentSILR = new avtSILRestriction(sil);
            currentSILR->SetTopSet(orig_SILR->GetTopSet());
            if (!currentSILR->SetFromCompatibleRestriction(orig_SILR))
            {
                debug4 << "Could not Set compatible restriction." << endl;
                currentSILR = orig_SILR;
            }
        }
        avtDataRequest_p newDS = new avtDataRequest(orig_DS, currentSILR);
        newDS->SetTimestep(currentTime);

        avtContract_p contract = 
            new avtContract(newDS, spec->GetPipelineIndex());
        if (parallelizingOverTime)
        {
            contract->SetOnDemandStreaming(true);
            contract->UseLoadBalancing(false);
        }
        else
            contract->NoStreaming();

        modified |= avtFilter::Update(contract);
        
        if (ExecutionSuccessful())
        {
            validTimes.push_back(currentTime);
        }
        else 
        {
            skippedTimes.push_back(currentTime);
        }

        avtCallback::ResetTimeout(5*60);
    } 

    //
    // It is possible that execution of some timesteps may have resulted
    // in an error condition.   This may be perfectly valid, so reset
    // the output's error flag now.  Derived types can set it again if
    // needed during CreateFinalOutput.
    //
    GetOutput()->GetInfo().GetValidity().ResetErrorOccurred();
    CreateFinalOutput();
    UpdateDataObjectInfo();

    // This barrier is basically for debugging to make sure we don't have
    // a parallel synchronization issue.
    Barrier();

    //
    // Ensure the pipeline is in the same state as when we began.
    //
    GetInput()->Update(spec);
    
    // 
    // Set the time information to be the time from the input, not from the
    // last execution.  This is particularly important when we parallelize
    // over time, since each MPI task will have a different cycle/time and then
    // we would get an improper merge exception.  Note that we don't just want
    // to copy the attributes over, since they may have been modified by the
    // filter.
    //
    GetOutput()->GetInfo().GetAttributes().SetCycle(
                             GetInput()->GetInfo().GetAttributes().GetCycle());
    GetOutput()->GetInfo().GetAttributes().SetTime(
                             GetInput()->GetInfo().GetAttributes().GetTime());

    return modified;
}


// ****************************************************************************
//  Method: avtTimeLoopFilter::CanDoTimeParallelization
//
//  Purpose:
//      Determines if we can do time parallelization. 
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2010
//
// ****************************************************************************

bool
avtTimeLoopFilter::CanDoTimeParallelization(void)
{
    return DataCanBeParallelizedOverTime() &&
           FilterSupportsTimeParallelization();
}


// ****************************************************************************
//  Method: avtTimeLoopFilter::DataCanBeParallelizedOverTime
//
//  Purpose:
//      Determines if data set can be processed over time in parallel.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2010
//
// ****************************************************************************

bool
avtTimeLoopFilter::DataCanBeParallelizedOverTime(void)
{
    // This test should ultimately be enhanced to detect new attributes that
    // indicate whether the data is small enough that it can be processed by
    // a single MPI task.  The infrastructure doesn't exist yet.
    return false;
}


// ****************************************************************************
//  Method: avtTimeLoopFilter::FinalizeTimeLoop
//
//  Purpose:  Sets the begin and end frames for the time loop.  Peforms error
//            checking on the values.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 27 08:02:03 PST 2005
//    Renamed from SetTimeLoop.  Removed args.  Check for set values and
//    use defaults as necessary.
//
// ****************************************************************************

void
avtTimeLoopFilter::FinalizeTimeLoop()
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
    if (startTime >= endTime)
    {
        std::string msg("Start time must be smaller than end time for " );
        msg += GetType();
        msg += ".\n";
        EXCEPTION1(ImproperUseException, msg);
    }

    nFrames = (int) ceil((((float)endTime -startTime))/(float)stride) + 1; 

    if (nFrames <= 1)
    {
        std::string msg(GetType());
        msg = msg +  " requires more than 1 frame, please correct start " + 
               "and end times and try again.";
        EXCEPTION1(ImproperUseException, msg);
    }

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
    actualEnd = startTime + nFrames *stride;
    if (actualEnd < endTime)
        actualEnd = endTime + stride;
}


// ****************************************************************************
//  Method: avtTimeLoopFilter::ReleaseData
//
//  Purpose:
//      Makes the output release any data that it has as a memory savings.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 29, 2004
//
// ****************************************************************************

void
avtTimeLoopFilter::ReleaseData(void)
{
    avtFilter::ReleaseData();    
}
