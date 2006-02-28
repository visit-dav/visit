// ************************************************************************* //
//                                  avtR2Fsum.C                              //
// ************************************************************************* //

#include <avtR2Fsum.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fsum constructor
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fsum::avtR2Fsum(int nb) : avtR2Foperator(nb, 0.)
{
    sum = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        sum[i] = 0.;
}


// ****************************************************************************
//  Method: avtR2Fsum destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fsum::~avtR2Fsum()
{
    delete [] sum;
}


// ****************************************************************************
//  Method: avtR2Fsum::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

void
avtR2Fsum::AddData(int b, float v)
{
    sum[b] += v;
}


// ****************************************************************************
//  Method: avtR2Fsum::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "sum", there is only
//      one pass.  So calculate the final sums and return them.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

float *
avtR2Fsum::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    SumFloatArrayAcrossAllProcessors(sum, rv, nBins);
    return rv;
}


