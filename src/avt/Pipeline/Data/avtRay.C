// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtRay.C                                  //
// ************************************************************************* //
#include <cstring>

#include <avtRay.h>

#include <avtSamplePointArbitrator.h>


avtSamplePointArbitrator   *avtRay::arbitrator = NULL;
bool                        avtRay::kernelBasedSampling = false;

static bool *staticAllFalse = NULL;
static int   staticNumSamps = -1;

// ****************************************************************************
//  Method: avtRay constructor
//
//  Arguments:
//      ns      The number of samples for the ray.
//      nv      The number of variables that we will sample.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 09:33:18 PST 2001
//    Added support for multiple variables.
//
//    Hank Childs, Mon Dec 31 13:13:44 PST 2001
//    Initialize numRuns.
//
//    Hank Childs, Tue Sep 22 08:56:19 PDT 2009
//    Use a static to initialize the validSample array more quickly.
//
//    Brad Whitlock, Tue Feb 14 13:09:55 PST 2017
//    Pass in preallocated buffers.
//
// ****************************************************************************

avtRay::avtRay(double *s, bool *vs, int ns, int nv)
{
    int  i;

    numSamples   = ns;
    numVariables = nv;

    // Get our buffers passed in.
    for (i = 0 ; i < numVariables ; i++)
    {
        sample[i] = s + i * numSamples;
    }
    for (i = numVariables ; i < AVT_VARIABLE_LIMIT ; i++)
    {
        sample[i] = NULL;
    }

    if (numSamples != staticNumSamps)
    {
        if (staticAllFalse != NULL)
            delete [] staticAllFalse;
        staticAllFalse = new bool[numSamples];
        for (i = 0 ; i < numSamples ; i++)
        {
            staticAllFalse[i] = false;
        }
        staticNumSamps = numSamples;
    }

    validSample = vs;
    memcpy(validSample, staticAllFalse, sizeof(bool)*numSamples);

    numValidSamples = 0;
    numRuns = 0;
}


// ****************************************************************************
//  Method: avtRay destructor
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 09:33:18 PST 2001
//    Added support for multiple variables.
//
//    Brad Whitlock, Tue Feb 14 13:09:55 PST 2017
//    Buffers are preallocated. We don't need to delete them.
//
// ****************************************************************************

avtRay::~avtRay()
{
}

void
avtRay::Reset()
{
    memcpy(validSample, staticAllFalse, sizeof(bool)*numSamples);
    numValidSamples = 0;
    numRuns = 0;
}

// ****************************************************************************
//  Method: avtRay::GetSample
//
//  Purpose:
//      Gets the value at the sample point.
//
//  Arguments:
//      index     The index of the sample point.
//      s         The value of the sample point.
//
//  Returns:      true if this is a valid sample, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 09:38:42 PST 2001
//    Added support for multiple variables.
//
// ****************************************************************************

bool
avtRay::GetSample(int index, double s[AVT_VARIABLE_LIMIT]) const
{
    if (index > numSamples || index < 0)
    {
        EXCEPTION2(BadIndexException, index, numSamples);
    }

    if (validSample[index])
    {
        for (int i = 0 ; i < numVariables ; i++)
        {
            s[i] = sample[i][index];
        }
    }

    return validSample[index];
}


// ****************************************************************************
//  Method: avtRay::SetArbitrator
//
//  Purpose:
//      Sets the arbitrator to use when setting up sample points.
//
//  Programmer: Hank Childs
//  Creation:   January 23, 2002
//
// ****************************************************************************

void
avtRay::SetArbitrator(avtSamplePointArbitrator *arb)
{
    arbitrator = arb;
}


// ****************************************************************************
//  Method: avtRay::SetKernelBasedSampling
//
//  Purpose:
//      Tells the ray whether or not we are doing kernel based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2006
//
// ****************************************************************************

void
avtRay::SetKernelBasedSampling(bool val)
{
    kernelBasedSampling = val;
}


// ****************************************************************************
//  Method: avtRay::GetFirstSampleOfLongestRun
//
//  Purpose:
//      Gets the first sample of the longest run.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2003
//
// ****************************************************************************

int
avtRay::GetFirstSampleOfLongestRun(void) const
{
    int longestRun = 0;
    int longestRunStartsAt = 0;
    int currentRun = 0;
    int currentRunStartsAt = 0;
    bool inRun = false;
    for (int i = 0 ; i < numSamples ; i++)
    {
        if (validSample[i])
        {
            if (inRun)
            {
                currentRun++;
                if (currentRun > longestRun)
                {
                    longestRun = currentRun;
                    longestRunStartsAt = currentRunStartsAt;
                }
            }
            else
            {
                inRun = true;
                currentRunStartsAt = i;
                currentRun = 1;
            }
        }
        else
        {
            inRun = false;
        }
    }

    if (longestRun > 0)
    {
        return longestRunStartsAt;
    }

    return -1;
}


// ****************************************************************************
//  Method: avtRay::GetLastSampleOfLongestRun
//
//  Purpose:
//      Gets the first sample of the longest run.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2003
//
// ****************************************************************************

int
avtRay::GetLastSampleOfLongestRun(void) const
{
    int longestRun = 0;
    int longestRunStartsAt = 0;
    int currentRun = 0;
    int currentRunStartsAt = 0;
    bool inRun = false;
    for (int i = 0 ; i < numSamples ; i++)
    {
        if (validSample[i])
        {
            if (inRun)
            {
                currentRun++;
                if (currentRun > longestRun)
                {
                    longestRun = currentRun;
                    longestRunStartsAt = currentRunStartsAt;
                }
            }
            else
            {
                inRun = true;
                currentRunStartsAt = i;
                currentRun = 1;
            }
        }
        else
        {
            inRun = false;
        }
    }

    if (longestRun > 0)
    {
        return longestRunStartsAt + longestRun-1;
    }

    return -1;
}


// ****************************************************************************
//  Method: avtRay::Finalize
//
//  Purpose:
//      Tells the ray that all of the samples have been set and that it should
//      prepare itself for queries from ray functions.  This is only needed at
//      this time for kernel based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2006
//
// ****************************************************************************

void
avtRay::Finalize(void)
{
    if (kernelBasedSampling)
    {
        for (int z = 0 ; z < numSamples ; z++)
        {
             if (!validSample[z])
                 continue;
             if (sample[numVariables-1][z] <= 0.)
                 continue;
             double denom = 1. / sample[numVariables-1][z];
             for (int i = 0 ; i < numVariables-1 ; i++)
             {
                 sample[i][z] *= denom;
             }
        }
    }
}


