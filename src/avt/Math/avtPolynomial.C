// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtPolynomial.h"
#include <math.h>
#include <visitstream.h>

// ****************************************************************************
//  Method:  avtPolynomial::avtPolynomial
//
//  Purpose:
//     Creates a polynomial of given degree. All coefficents are initialized 
//     to zero.
//
//  Arguments:
//      degree the degree of the polynomial (coeff length-1)
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
avtPolynomial::avtPolynomial(int degree)
: degree(degree)
{
    coeffs.resize(degree+1);
    for(int i=0; i <= degree; i++)
        this->coeffs[i] = 0.0;
}

// ****************************************************************************
//  Method:  avtPolynomial::avtPolynomial
//
//  Purpose:
//     Creates a polynomial with passed coefficents. The resulting polynomial
//     will have degree = ceoffs.size()-1.
//
//  Arguments:
//      coeffs vector of polynomial coeffients
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
avtPolynomial::avtPolynomial(const doubleVector& coeffs)
: degree(coeffs.size()-1) , coeffs(coeffs)
{

}


// ****************************************************************************
//  Method:  avtPolynomial::~avtPolynomial
//
//  Purpose:
//     Polynomial destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
avtPolynomial::~avtPolynomial()
{;}

// ****************************************************************************
//  Method:  avtPolynomial::avtPolynomial(avtPolynomial)
//
//  Purpose:
//     Polynomial copy constructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
avtPolynomial::avtPolynomial(const avtPolynomial &poly)
: degree(poly.degree), coeffs(poly.coeffs)
{;}

// ****************************************************************************
//  Method:  avtPolynomial::operator=(avtPolynomial)
//
//  Purpose:
//     Polynomial assignment operator.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
avtPolynomial &
avtPolynomial::operator=(const avtPolynomial &poly)
{
    if(this != &poly)
    {
        degree = poly.degree;
        coeffs = poly.coeffs;
    }
    return *this;
}

// ****************************************************************************
//  Method:  avtPolynomial::Eval(avtPolynomial)
//
//  Purpose:
//     Evaluates the polynomial at x using Horner's method.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
double
avtPolynomial::Eval(double x) const
{
    // loop index
    int i = 0;
    // use horner's method for O(n) eval
    double res = coeffs[degree];
    if(degree > 0)
        for( i = degree - 1; i>=0; --i)
            res = res * x + coeffs[i];
    return res;
}

// ****************************************************************************
//  Method:  avtPolynomial::Eval(avtPolynomial)
//
//  Purpose:
//     Evaluates the polynomial at set of points using Horner's method.
//     The result is placed in res_vec.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
void
avtPolynomial::Eval(const std::vector<double> &in_vec, 
                          std::vector<double> &res_vec) const
{
    
    int n_samples = in_vec.size();
    res_vec.resize(n_samples);
    // loop indices
    int i = 0;
    int j = 0;
    
    double res = 0.0;
    double x   = 0.0;
    for(j =0; j< n_samples;j++)
    {
        x = in_vec[j];
        // use horner's method for O(n) eval
        res = coeffs[degree];
        if(degree > 0)
            for( i = degree - 1; i>=0; --i)
                res = res * x + coeffs[i];

        res_vec[j] = res;
    }
}


// ****************************************************************************
//  Method:  avtPolynomial::ToString
//
//  Purpose:
//     Creates a human readable string of the polynomial.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************
std::string 
avtPolynomial::ToString() const
{
    int i=0;
    // construct with ostringstream
    std::ostringstream oss;
    bool prev = false;    
    for(i = degree; i>=0; --i)
    {
        if(coeffs[i] != 0.0)
        {
            // add +
            if(prev && coeffs[i] > 0.0)
                oss << " + ";
            if(prev && coeffs[i] < 0.0)
                oss << " - ";

            // append value
            if(i<1 || coeffs[i]!=1.0)
                oss << fabs(coeffs[i]);
           
            // append *x^i
            if(i>1)
                oss << "x^" << i ;
            else if(i==1)
                oss << "x";
            prev = true;
        }
        else if(i == 0 && !prev)
        {   
            oss << "0.0";
        }
    }
    return oss.str();
}



