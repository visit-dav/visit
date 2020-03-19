// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtRay.h                                  //
// ************************************************************************* //

#ifndef AVT_RAY_H
#define AVT_RAY_H

#include <pipeline_exports.h>

#include <avtCellTypes.h>
#include <avtSamplePointArbitrator.h>

#include <BadIndexException.h>


// ****************************************************************************
//  Class: avtRay
//
//  Purpose:
//      Contains the sample points for a ray and methods to convert those
//      sample points to a color.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Data Members:
//     sample             These are the actual sample points.
//     validSample        Indicates whether there are valid samples at that 
//                        index.
//     numVariables       How many variables are we sampling?
//     numSamples         The maximum number of samples this data structure
//                        can store.
//     numValidSamples    The number of samples that contain valid sample
//                        points.  This is useful for byte packing routines.
//     numRuns            The number of runs (consecutive sample points) in
//                        the ray.  This includes runs of length 1.  This is
//                        useful for byte packing routines.
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 09:25:54 PST 2001
//    Added support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:13:44 PST 2001
//    Added bookkeeping to keep track of the number of runs.
//
//    Hank Childs, Wed Jan 23 09:49:52 PST 2002
//    Add an arbitrator.
//
//    Hank Childs, Sun Jan  1 12:33:37 PST 2006
//    Added support for kernel based sampling.
//
//    Hank Childs, Mon Sep 11 15:10:50 PDT 2006
//    Added friend status for the avtIntegrationRF.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Sun Aug 31 10:53:59 PDT 2008
//    Added friend status to avtPhong.
//
// ****************************************************************************

class PIPELINE_API avtRay
{
    //
    // avtVolume and all of the derived types of avtRay are made friends for
    // performance reasons.
    //
    friend class                  avtAverageValueRF;
    friend class                  avtCompositeRF;
    friend class                  avtDistanceToValueRF;
    friend class                  avtIntegrationRF;
    friend class                  avtMaximumIntensityProjectionRF;
    friend class                  avtMIPRangeRF;
    friend class                  avtPhong;
    friend class                  avtVolume;

  public:
                                  avtRay(double *s, bool *vs, int, int);
    virtual                      ~avtRay();

    void                          Reset();

    inline void                   SetSample(const int &,
                                            const double[AVT_VARIABLE_LIMIT]);
    inline void                   SetSamples(int start, int end,
                                          const double (*)[AVT_VARIABLE_LIMIT]);
    inline void                   UpdateNumberOfRuns(int);
    bool                          GetSample(int, double [AVT_VARIABLE_LIMIT])
                                       const;
    inline int                    GetNumberOfSamples(void) const
                                                        { return numSamples; };
    inline int                    GetFirstSample(void) const;
    inline int                    GetLastSample(void) const;
    int                           GetFirstSampleOfLongestRun(void) const;
    int                           GetLastSampleOfLongestRun(void) const;
             
    void                          Finalize(void);

    static void                   SetArbitrator(avtSamplePointArbitrator *);
    static void                   SetKernelBasedSampling(bool);

  protected:
    double                       *sample[AVT_VARIABLE_LIMIT];
    bool                         *validSample;
    int                           numVariables;
    int                           numSamples;
    int                           numValidSamples;
    int                           numRuns;
    static avtSamplePointArbitrator *arbitrator;
    static bool                   kernelBasedSampling;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtRay(const avtRay &) {;};
    avtRay              &operator=(const avtRay &) { return *this; };
};


// ****************************************************************************
//  Method: avtRay::SetSample
//
//  Purpose:
//      Sets a sample along this ray.
//
//  Arguments:
//      si      The sample index.
//      val     The value of the sample point.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:36:28 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:13:44 PST 2001
//    Keep track of the number of runs.
//
//    Hank Childs, Wed Jan 23 09:46:24 PST 2002
//    Make each sample point be the best contributor from its "region".
//
//    Hank Childs, Sun Jan  1 12:34:29 PST 2006
//    Added support for kernel based sampling.
//
//    Hank Childs, Tue Jan 15 14:49:26 PST 2008
//    Fix bad indexing bug with arbitrator.
//
// ****************************************************************************

inline void
avtRay::SetSample(const int &si, const double val[AVT_VARIABLE_LIMIT])
{
    if (si < numSamples && si >= 0)
    {
        if (kernelBasedSampling)
        {
            if (validSample[si])
            {
                for (int i = 0 ; i < numVariables ; i++)
                    sample[i][si] += val[i];
            }
            else
            {
                for (int i = 0 ; i < numVariables ; i++)
                    sample[i][si] = val[i];
            }
        }
        else
        {
            bool shouldOverwrite = true;
            if (validSample[si] && arbitrator != NULL)
            {
                int index = arbitrator->GetArbitrationVariable();
                shouldOverwrite = arbitrator->ShouldOverwrite(sample[index][si],
                                                              val[index]);
            }
            if (shouldOverwrite)
            {
                for (int i = 0 ; i < numVariables ; i++)
                {
                    sample[i][si] = val[i];
                }
            }
        }
        if (!validSample[si])
        {
            validSample[si] = true;
            numValidSamples++;
            UpdateNumberOfRuns(si);
        }
    }
    else
    {
        EXCEPTION2(BadIndexException, si, numSamples);
    }

}


// ****************************************************************************
//  Method: avtRay::SetSamples
//
//  Purpose:
//      Sets a row of continuous samples.
//
//  Arguments:
//      start     The first index to set.
//      end       The last index to set.
//      samps     The values at each sample point.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 11:36:28 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:13:44 PST 2001
//    Keep track of the number of runs.
//
//    Hank Childs, Wed Jan 23 09:46:24 PST 2002
//    Make each sample point be the best contributor from its "region".
//
// ****************************************************************************

inline void
avtRay::SetSamples(int start, int end,
                   const double (*samps)[AVT_VARIABLE_LIMIT])
{
    if (start >= 0 && end < numSamples)
    {
        if(arbitrator != NULL)
        {
            int index = arbitrator->GetArbitrationVariable();
            for (int i = start ; i <= end ; i++)
            {
                bool shouldOverwrite = true;
                if (validSample[i])
                {
                    shouldOverwrite = arbitrator->ShouldOverwrite(sample[index][i],
                                                                  samps[i-start][index]);
                }
                if (shouldOverwrite)
                {
                    for (int j = 0 ; j < numVariables ; j++)
                    {
                        sample[j][i] = samps[i-start][j];
                    }
                }
                if (!validSample[i])
                {
                    validSample[i] = true;
                    numValidSamples++;
                    UpdateNumberOfRuns(i);
                }
            }
        }
        else
        {
            for (int i = start ; i <= end ; i++)
            {
                for (int j = 0 ; j < numVariables ; j++)
                    sample[j][i] = samps[i-start][j];

                if (!validSample[i])
                {
                    validSample[i] = true;
                    numValidSamples++;
                    UpdateNumberOfRuns(i);
                }
            }
        }
    }
    else
    {
        if (start < 0)
        {
            EXCEPTION2(BadIndexException, start, numSamples);
        }
        else
        {
            EXCEPTION2(BadIndexException, end, numSamples);
        }
    }
}


// ****************************************************************************
//  Method: avtRay::UpdateNumberOfRuns
//
//  Purpose:
//      Updates the number of runs in the ray.  A "run" is a sequence of
//      consecutive sample points in the ray.  This includes runs of one
//      sample point.
//
//  Arguments:
//      ind      The index of the sample point that just became valid for the 
//               first time.
//
//  Programmer:  Hank Childs
//  Creation:    December 31, 2001
//
// ****************************************************************************

inline void
avtRay::UpdateNumberOfRuns(int ind)
{
    bool validOnLeft  = (ind-1 >= 0 ? validSample[ind-1] : false);
    bool validOnRight = (ind+1 < numSamples ? validSample[ind+1] : false);

    //
    // If it was valid on the left and on the right, then we have bridged two
    // runs -- reflect that in our count.
    //
    if (validOnLeft && validOnRight)
    {
        numRuns--;
    }

    //
    // If it was not valid on the left and not valid on the right, then we 
    // have created a new run -- reflect that in our count.
    //
    if (!validOnLeft && !validOnRight)
    {
        numRuns++;
    }

    //
    // else it was valid on one end and not the other, so we have extended a
    // run -- this does not affect our counts.
    //
}


// ****************************************************************************
//  Method: avtRay::GetFirstSample
//
//  Purpose:
//      Gets the first valid sample.
//
//  Returns:    The index of the first valid sample, -1 if no valid samples.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

inline int
avtRay::GetFirstSample(void) const
{
    if (numValidSamples <= 0)
        return -1;

    for (int i = 0 ; i < numSamples ; i++)
        if (validSample[i])
            return i;

    // should never get here.
    return -1;
}


// ****************************************************************************
//  Method: avtRay::GetLastSample
//
//  Purpose:
//      Gets the last valid sample.
//
//  Returns:    The index of the last valid sample, -1 if no valid samples.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

inline int
avtRay::GetLastSample(void) const
{
    if (numValidSamples <= 0)
        return -1;

    for (int i = numSamples-1 ; i >= 0 ; i--)
        if (validSample[i])
            return i;

    // should never get here.
    return -1;
}


#endif


