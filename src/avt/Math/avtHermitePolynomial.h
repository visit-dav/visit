// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_HERMITE_POLYNOMIAL_H
#define AVT_HERMITE_POLYNOMIAL_H
#include <math_exports.h>
#include "avtPolynomial.h"


// ****************************************************************************
//  Class:  avtHermitePolynomial
//
//  Purpose:
//    Encapsualtes a hermite polynomial.
//    The constructor generates the polynomial using the recurrence relation:
//       H(x|n+1) = 2x*H(x|n) - 2n*H(x|n-1)
//    Where:
//       H(x|0) = 1
//       H(x|1) = 2x
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
//  Modifications:
//
// ****************************************************************************

class MATH_API avtHermitePolynomial : public avtPolynomial
{
  public:
    avtHermitePolynomial(int n);
    virtual ~avtHermitePolynomial();
};



#endif
