// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtShapeletBasis1D.h>
#include <avtHermitePolynomial.h>
#include <avtFactorial.h>
#include <math.h>
 using namespace std;

// ****************************************************************************
//  Method:  avtShapeletBasis1D::avtShapeletBasis1D
//
//  Purpose:
//     Constructs a sampled 1D Shapelet Basis function with given paramters.
//
//  Arguments:
//     beta      characteristic scale
//     n         basis order
//     length    sample length
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis1D::avtShapeletBasis1D(double beta,
                                       int n,
                                       int length)
: beta(beta), n(n), length(length)
{
    double sqrt_pi = 1.77245385;
    double e       = 2.71828183;
    
    doubleVector samples;
    samples.resize(length);
    values.resize(length);

    // create the proper hermite poly
    avtHermitePolynomial hermite(n);

    // create samples
    double val = -length/2;
    for(int i=0;i<length;i++)
    {
        samples[i] = val/ beta;
        val++;
    }

    // eval the hermite poly over the range
    hermite.Eval(samples,values);
    
    // apply the gaussian mult to transform the hermite result into a
    // shaplet
    
    double pre=sqrt(beta*pow(2.0,(double)n)*sqrt_pi*avtFactorial::Eval(n));
    pre = 1.0 / pre;
    double exp = 0;
    for(int i=0;i<length;i++)
    {
        exp = pow(e,samples[i] * samples[i]/-2.0);
        values[i] = pre * values[i] * exp;
    }
}

// ****************************************************************************
//  Method:  avtShapeletBasis1D::~avtShapeletBasis1D
//
//  Purpose:
//     Shapelet 1D Basis destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis1D::~avtShapeletBasis1D()
{;}


