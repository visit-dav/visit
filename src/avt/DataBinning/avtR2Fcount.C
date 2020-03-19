// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                  avtR2Fcount.C                            //
// ************************************************************************* //

#include <avtR2Fcount.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fcount constructor
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jan 10 16:03:03 EST 2008
//
// ****************************************************************************

avtR2Fcount::avtR2Fcount(int nb) : avtR2Foperator(nb, 0.)
{
    count = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        count[i] = 0.;
}


// ****************************************************************************
//  Method: avtR2Fcount destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jan 10 16:03:19 EST 2008
//
// ****************************************************************************

avtR2Fcount::~avtR2Fcount()
{
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Fcount::AddData
//
//  Purpose:
//      Adds a single data point to the operator.  Ignores the value of the
//      data and simply increments the count.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jan 10 16:03:38 EST 2008
//
// ****************************************************************************

void
avtR2Fcount::AddData(int b, float)
{
    count[b]++;
}


// ****************************************************************************
//  Method: avtR2Fcount::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "count", there is only
//      one pass.  So calculate the final count and return it.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jan 10 16:04:02 EST 2008
//
// ****************************************************************************

float *
avtR2Fcount::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    SumFloatArrayAcrossAllProcessors(count, rv, nBins);
    return rv;
}


