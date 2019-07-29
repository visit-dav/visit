// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtR2Fminimum.C                             //
// ************************************************************************* //

#include <avtR2Fminimum.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fminimum constructor
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

avtR2Fminimum::avtR2Fminimum(int nb, double uv) : avtR2Foperator(nb, uv)
{
    min = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        min[i] = FLT_MAX;
}


// ****************************************************************************
//  Method: avtR2Fminimum destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtR2Fminimum::~avtR2Fminimum()
{
    delete [] min;
}


// ****************************************************************************
//  Method: avtR2Fminimum::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

void
avtR2Fminimum::AddData(int b, float v)
{
    if (v < min[b])
        min[b] = v;
}


// ****************************************************************************
//  Method: avtR2Fminimum::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "minimum", there is only
//      one pass.  So calculate the final minimums and return them.  The caller
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
avtR2Fminimum::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    UnifyMinimumFloatArrayAcrossAllProcessors(min, rv, nBins);
    for (int i = 0 ; i < nBins ; i++)
        rv[i] = (rv[i] == FLT_MAX ? undefinedVal : rv[i]);
    return rv;
}


