// ************************************************************************* //
//                       avtTimeLoopFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_FILTER_H
#define AVT_TIME_LOOP_FILTER_H

#include <pipeline_exports.h>
#include <vectortypes.h>
#include <avtFilter.h>



// ****************************************************************************
//  Method: avtTimeLoopFilter
//
//  Purpose:
//    An abstract filter that loops over timesteps in the Update portion.
//    Allows derived combine output from different timesteps into a single
//    output (avtDataObject, avtDataset, avtDataTree). 
//
//    It is up to the derived types to save necessary data for each timestep
//    and to properly combine into an output during CreateFinalOutput.
//
//    Derived types must report whether or not a given exection cycle was
//    successful.  This parent class keeps track of timesteps that executed
//    successfully and those that did not.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   December 29, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005
//    Added SetStartTime, SetEndTime, SetStride and FinalizeTimeLoop.
//  
// ****************************************************************************

class PIPELINE_API avtTimeLoopFilter : virtual public avtFilter
{
  public:
                                        avtTimeLoopFilter();
    virtual                            ~avtTimeLoopFilter();

    virtual bool                        Update(avtPipelineSpecification_p);
    virtual void                        ReleaseData(void);

    void                                SetTimeLoop(int b, int e, int s)
                                            { startTime = b; 
                                              endTime = e; 
                                              stride = s; };
    void                                SetStartFrame(int b)
                                            { startTime = b; };
    void                                SetEndFrame(int e)
                                            { endTime = e; };
    void                                SetStride(int s)
                                            { stride = s; };

  protected:
    intVector                           validTimes;
    intVector                           skippedTimes;
    int                                 currentTime;
    avtSILRestriction_p                 currentSILR;
    std::string                         errorMessage;

    virtual void                        CreateFinalOutput(void) = 0;
    virtual bool                        ExecutionSuccessful(void) = 0;

  private:
    int                                 startTime;
    int                                 endTime;
    int                                 stride;
    int                                 nFrames;
    int                                 actualEnd;
    void                                FinalizeTimeLoop(void);
};


#endif


