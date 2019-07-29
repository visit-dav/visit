// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtExecuteThenTimeLoopFilter.h                           //
// ************************************************************************* //

#ifndef AVT_EXECUTE_THEN_TIME_LOOP_FILTER_H
#define AVT_EXECUTE_THEN_TIME_LOOP_FILTER_H

#include <expression_exports.h>

#include <vectortypes.h>

#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Method: avtExecuteThenTimeLoopFilter
//
//  Purpose:
//    An abstract filter that is similar to the time loop filter.  It
//    allows the pipeline to execute for the current time slice, and then
//    starts iterating over other time slices.  A key difference between
//    this filter and the avtTimeLoopFilter is that this filter treats
//    the current time slice as "special" ... allowing it to examine the
//    current time slice as a way to direct the analysis when it does
//    time iteration.
//
//    The other key difference between the execute-then-time-loop-filter
//    with the avtTimeLoopFilter is that it doesn't use upstream portion of
//    the pipeline when executing.  That is, if the user added a slice 
//    operator, the slice operator will be ignored for the time iteration
//    portion, with the data set retrieval occurring directly from the
//    database (with an EEF).
//   
//  Programmer: Hank Childs
//  Creation:   January 24, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 19:19:41 PST 2009
//    Added infrastructure for using the contract for the first execution.
//
//    Kathleen Bonnell, Thu Mar 26 08:13:26 MST 2009 
//    Changed API from PIPELINE to EXPRESSION.
//
// ****************************************************************************

class EXPRESSION_API avtExecuteThenTimeLoopFilter 
    : virtual public avtDatasetToDatasetFilter
{
  public:
                                        avtExecuteThenTimeLoopFilter();
    virtual                            ~avtExecuteThenTimeLoopFilter();

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
    virtual void                        Execute(void);
    virtual void                        InspectPrincipalData(void) {;};
    virtual void                        Iterate(int, avtDataTree_p) = 0;
    virtual void                        Finalize(void) = 0;

    void                                FinalizeTimeLoop(void);
    virtual avtContract_p               ModifyContract(avtContract_p c)
                                           { SetContract(c); return c; };

    void                                SetContract(avtContract_p c)
                                               { origContract = c; };
  private:
    int                                 startTime;
    int                                 endTime;
    int                                 stride;
    int                                 nFrames;
    int                                 actualEnd;
    avtContract_p                       origContract;
};


#endif


