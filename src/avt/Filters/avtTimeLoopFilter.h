// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtTimeLoopFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_FILTER_H
#define AVT_TIME_LOOP_FILTER_H

#include <filters_exports.h>
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
//    Hank Childs, Wed Dec 15 14:30:42 PST 2010
//    Add support for parallelizing over time.
//
//    Dave Pugmire, Tue Jul 17 11:52:34 EDT 2012
//    Added ability to make multiple passes over the time series.
//
//   Dave Pugmire, Thu May 23 10:56:50 EDT 2013
//   Rename the loop initialization method. Add Cycle/Times queries.
//
// ****************************************************************************

class AVTFILTERS_API avtTimeLoopFilter : virtual public avtFilter
{
  public:
                                        avtTimeLoopFilter();
    virtual                            ~avtTimeLoopFilter();

    virtual bool                        Update(avtContract_p);
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
    void                                SetIncludeLastTime(bool val) { includeLastTime = val;}
    int                                 GetStartTime() const {return startTime;}
    int                                 GetEndTime() const {return endTime;}
    int                                 GetNFrames() const {return nFrames;}
    bool                                GetIncludeLastTime() const { return includeLastTime;}


  protected:
    intVector                           validTimes;
    intVector                           skippedTimes;
    int                                 currentTime;
    avtSILRestriction_p                 currentSILR;
    std::string                         errorMessage;

    virtual void                        SetNumberOfIterations( int i) {nIterations = i;}
    virtual int                         GetNumberOfIterations() {return nIterations;}
    virtual int                         GetIteration() {return timeLoopIter;}
    virtual int                         GetFrame() {return frameIter;}
    virtual int                         GetTotalNumberOfTimeSlicesForRank();
    virtual std::vector<int>            GetCyclesForRank();
    virtual std::vector<double>         GetTimesForRank();

    virtual bool                        NeedCurrentTimeSlice() {return true;}
    virtual void                        CreateFinalOutput(void) = 0;
    virtual bool                        ExecutionSuccessful(void) = 0;


  private:
    int                                 startTime;
    int                                 endTime;
    int                                 stride;
    int                                 nFrames;
    int                                 frameIter;
    int                                 actualEnd;
    int                                 nIterations;
    int                                 timeLoopIter;
    bool                                includeLastTime;
    bool                                parallelizingOverTime;

    virtual bool                        RankOwnsTimeSlice(int t);
    virtual void                        PreLoopInitialize(void) {}
  protected:
    virtual void                        InitializeTimeLoop(void);

    // Asks whether we have decided to do time parallelization.  This 
    // is for derived types to understand what mode we're in.
    bool                                ParallelizingOverTime(void)
                                          { return parallelizingOverTime; };

    // Asks whether the derived type supports time parallelization.
    // If a filter does support time parallelization, it should re-implement
    // this method to return true.
    virtual bool                        FilterSupportsTimeParallelization(void)
                                          { return false; };

    // Asks whether the filter needs to look at the entire data set to
    // perform its function when parallelizing over time.  For example, some
    // picks can parallelize over time by only reading one domain.
    virtual bool                        OperationNeedsAllData(void) 
                                          { return true; };
    
    // Asks whether the data can be parallelized over time.
    virtual bool                         DataCanBeParallelizedOverTime(void);

  private:
    // Asks whether it is possible to do parallelization over time.
    // This is for internal decision making.
    bool                                CanDoTimeParallelization(void);

};


#endif


