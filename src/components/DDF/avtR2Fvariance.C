// ************************************************************************* //
//                                avtR2Fvariance.C                             //
// ************************************************************************* //

#include <avtR2Fvariance.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fvariance constructor
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fvariance::avtR2Fvariance(int nb, double uv) : avtR2Foperator(nb, uv)
{
    running_total_ave    = new double[nb];
    running_total_variance = new double[nb];
    count                = new int[nb];
    for (int i = 0 ; i < nb ; i++)
    {
        running_total_ave[i]    = 0.;
        running_total_variance[i] = 0.;
        count[i]                = 0;
    }
    pass = 0;
}


// ****************************************************************************
//  Method: avtR2Fvariance destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fvariance::~avtR2Fvariance()
{
    delete [] running_total_ave;
    delete [] running_total_variance;
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Fvariance::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

void
avtR2Fvariance::AddData(int b, float v)
{
    if (pass == 0)
    {
        running_total_ave[b] += v;
        count[b]++;
    }
    else
    {
        float diff = (v-running_total_ave[b]);
        running_total_variance[b] += diff*diff;
    }
}


// ****************************************************************************
//  Method: avtR2Fvariance::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the first pass, calculate the average.
//      In the second pass, calculate the standard deviation.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

float *
avtR2Fvariance::FinalizePass(int pass)
{
    float *rv = NULL;
    if (pass == 0)
    {
        double *rt2  = new double[nBins];
        int    *cnt2 = new int[nBins];
        SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
        SumDoubleArrayAcrossAllProcessors(running_total_ave, rt2, nBins);
        for (int i = 0 ; i < nBins ; i++)
        {
            if (cnt2[i] > 0)
                running_total_ave[i] = rt2[i] / cnt2[i];
            else
                running_total_ave[i] = undefinedVal;
        }
        delete [] rt2;
        delete [] cnt2;
    }
    else
    {
        rv           = new float[nBins];
        double *rt2  = new double[nBins];
        int    *cnt2 = new int[nBins];
        SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
        SumDoubleArrayAcrossAllProcessors(running_total_variance, rt2, nBins);
        for (int i = 0 ; i < nBins ; i++)
        {
            if (cnt2[i] > 0)
                rv[i] = rt2[i] / cnt2[i];
            else
                rv[i] = undefinedVal;
        }
        delete [] rt2;
        delete [] cnt2;
    }

    this->pass++;
    return rv;
}


