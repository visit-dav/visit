// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                   avtR2Fpdf.C                             //
// ************************************************************************* //

#include <avtR2Fpdf.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fpdf constructor
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2010
//
// ****************************************************************************

avtR2Fpdf::avtR2Fpdf(int nb) : avtR2Foperator(nb, 0.)
{
    pdf = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        pdf[i] = 0.;
}


// ****************************************************************************
//  Method: avtR2Fpdf destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2010
//
// ****************************************************************************

avtR2Fpdf::~avtR2Fpdf()
{
    delete [] pdf;
}


// ****************************************************************************
//  Method: avtR2Fpdf::AddData
//
//  Purpose:
//      Adds a single data point to the operator.  Ignores the value of the
//      data and simply increments the pdf.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2010
//
// ****************************************************************************

void
avtR2Fpdf::AddData(int b, float)
{
    pdf[b]++;
}


// ****************************************************************************
//  Method: avtR2Fpdf::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "pdf", there is only
//      one pass.  So calculate the final pdf and return it.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2010
//
// ****************************************************************************

float *
avtR2Fpdf::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    SumFloatArrayAcrossAllProcessors(pdf, rv, nBins);
    double nvals = 0;
    int i;
    for (i = 0 ; i < nBins ; i++)
        nvals += rv[i];
    for (i = 0 ; i < nBins ; i++)
        rv[i] /= nvals;
    return rv;
}


