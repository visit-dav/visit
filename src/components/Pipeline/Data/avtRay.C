// ************************************************************************* //
//                                 avtRay.C                                  //
// ************************************************************************* //

#include <avtRay.h>

#include <avtSamplePointArbitrator.h>


avtSamplePointArbitrator   *avtRay::arbitrator = NULL;


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
// ****************************************************************************

avtRay::avtRay(int ns, int nv)
{
    int  i;

    numSamples   = ns;
    numVariables = nv;
    for (i = 0 ; i < numVariables ; i++)
    {
        sample[i] = new float[numSamples];
    }
    for (i = numVariables ; i < AVT_VARIABLE_LIMIT ; i++)
    {
        sample[i] = NULL;
    }
    validSample = new bool[numSamples];
    for (i = 0 ; i < numSamples ; i++)
    {
        validSample[i] = false;
    }
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
// ****************************************************************************

avtRay::~avtRay()
{
    if (validSample != NULL)
    {
        delete [] validSample;
        validSample = NULL;
    }
    for (int i = 0 ; i < AVT_VARIABLE_LIMIT ; i++)
    {
        if (sample[i] != NULL)
        {
            delete [] sample[i];
            sample[i] = NULL;
        }
    }
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
avtRay::GetSample(int index, float s[AVT_VARIABLE_LIMIT]) const
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


