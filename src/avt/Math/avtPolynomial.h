// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_POLYNOMIAL_H
#define AVT_POLYNOMIAL_H
#include <math_exports.h>
#include <vectortypes.h>
#include <string>

// ****************************************************************************
//  Class:  avtPolynomial
//
//  Purpose:
//    Encapsulates an arbitrary polynomial. Provides a polynomial eval 
//    function that uses Horner's O(n) method.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
//  Modifications:
//
// ****************************************************************************

class MATH_API avtPolynomial
{
  public:
    // constructors
    avtPolynomial(int degree);
    avtPolynomial(const doubleVector &coeff);
    avtPolynomial(const avtPolynomial&);
    // assignment operator
    avtPolynomial &operator=(const avtPolynomial&);
    virtual ~avtPolynomial();

    int     Degree() const     {return degree;}
    // coeff access
    double &Coefficient(int i) {return coeffs[i];}  
    double &operator[](int i)  {return coeffs[i];}  
   
    // efficent eval
    double Eval(double) const;
    void   Eval(const doubleVector&, doubleVector&) const;
    
    // pretty string rep
    std::string ToString() const;
    
 protected:
    int          degree;
    doubleVector coeffs;
      
};


#endif
