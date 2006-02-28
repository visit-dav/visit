// ************************************************************************* //
//                               avtR2Fmaximum.C                             //
// ************************************************************************* //

#include <avtR2Fmaximum.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fmaximum constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:22:19 PST 2006
//    Add undefinedVal.
//
// ****************************************************************************

avtR2Fmaximum::avtR2Fmaximum(int nb, double uv) : avtR2Foperator(nb, uv)
{
    max = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        max[i] = -FLT_MAX;
}


// ****************************************************************************
//  Method: avtR2Fmaximum destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtR2Fmaximum::~avtR2Fmaximum()
{
    delete [] max;
}


// ****************************************************************************
//  Method: avtR2Fmaximum::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

void
avtR2Fmaximum::AddData(int b, float v)
{
    if (v > max[b])
        max[b] = v;
}


// ****************************************************************************
//  Method: avtR2Fmaximum::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "maximum", there is only
//      one pass.  So calculate the final maximums and return them.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:28:11 PST 2006
//    Use the undefined val.
//
// ****************************************************************************

float *
avtR2Fmaximum::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    UnifyMaximumFloatArrayAcrossAllProcessors(max, rv, nBins);
    for (int i = 0 ; i < nBins ; i++)
        rv[i] = (rv[i] == -FLT_MAX ? undefinedVal : rv[i]);
    return rv;
}


