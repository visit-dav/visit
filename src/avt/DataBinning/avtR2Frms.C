// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtR2Frms.C                               //
// ************************************************************************* //

#include <avtR2Frms.h>

#include <avtParallel.h>
#include <math.h>


// ****************************************************************************
//  Method: avtR2Frms constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

avtR2Frms::avtR2Frms(int nb, double uv) : avtR2Foperator(nb, uv)
{
    running_total = new double[nb];
    count         = new int[nb];
    for (int i = 0 ; i < nb ; i++)
    {
        running_total[i] = 0.;
        count[i]         = 0;
    }
}


// ****************************************************************************
//  Method: avtR2Frms destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
// ****************************************************************************

avtR2Frms::~avtR2Frms()
{
    delete [] running_total;
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Frms::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
// ****************************************************************************

void
avtR2Frms::AddData(int b, float v)
{
    running_total[b] += v*v;
    count[b]++;
}


// ****************************************************************************
//  Method: avtR2Frms::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "rms", there is only
//      one pass. The array  running_total contains:
//          v = (x0^2 + x1^2 + ... + xN^2)
//      To finalize calculate rms = sqrt(v/N) for each bin.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

float *
avtR2Frms::FinalizePass(int pass)
{
    float  *rv   = new float[nBins];
    double *rt2  = new double[nBins];
    int    *cnt2 = new int[nBins];
    SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
    SumDoubleArrayAcrossAllProcessors(running_total, rt2, nBins);
    for (int i = 0 ; i < nBins ; i++)
    {
        if (cnt2[i] > 0)
            rv[i] = sqrt(rt2[i] / cnt2[i]);
        else
            rv[i] = undefinedVal;
    }
    delete [] rt2;
    delete [] cnt2;

    return rv;
}


