// ************************************************************************* //
//                          avtTimeLoopFilter.C                              //
// ************************************************************************* //

#include <avtTimeLoopFilter.h>

#include <avtCallback.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>



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
// ****************************************************************************

avtTimeLoopFilter::avtTimeLoopFilter()
{
    startTime = -1;
    endTime = -1;
    stride = -1; 
    nFrames = 0;
    actualEnd = 0;
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
//    creates appropriate avtDataSpecification and avtPipelineSpecification
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
// ****************************************************************************

bool
avtTimeLoopFilter::Update(avtPipelineSpecification_p spec)
{
    int i;
    bool modified = false;
    avtDataSpecification_p orig_DS = spec->GetDataSpecification();
    avtSILRestriction_p orig_SILR = orig_DS->GetRestriction();

    FinalizeTimeLoop();

    for (i = startTime; i < actualEnd; i+= stride)
    {
        if (i < endTime)
           currentTime = i; 
        else 
           currentTime = endTime; 

        avtSIL *sil = GetInput()->GetTerminatingSource()->GetSIL(currentTime);
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
        avtDataSpecification_p newDS = new avtDataSpecification(orig_DS, currentSILR);
        newDS->SetTimestep(currentTime);

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, spec->GetPipelineIndex());

        modified |= avtFilter::Update(pspec);
        
        if (ExecutionSuccessful())
        {
            validTimes.push_back(currentTime);
        }
        else 
        {
            skippedTimes.push_back(currentTime);
        }
    } 

    //
    // It is possible that execution of some timesteps may have resulted
    // in an error condition.   This may be perfectly valid, so reset
    // the output's error flag now.  Derived types can set it again if
    // needed during CreateFinalOutput.
    //
    GetOutput()->GetInfo().GetValidity().ResetErrorOccurred();
    CreateFinalOutput();
    RefashionDataObjectInfo();

    return modified;
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
