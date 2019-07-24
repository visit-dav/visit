// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtHermitePolynomial.h"
#include <math.h>
#include <sstream>
#include <iostream>

using namespace std;

// ****************************************************************************
//  Method:  avtHermitePolynomial::avtHermitePolynomial
//
//  Purpose:
//     Constructs a hermite polynomial of order n.
//
//  Arguments:
//     n  desired hermite polynomial order
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtHermitePolynomial::avtHermitePolynomial(int n)
: avtPolynomial(n)
{
    if(n == 0)      // H(0|1) = 1
    {
        Coefficient(0) = 1.0;
    }
    else if(n == 1) // H(x|1) = 2x
    {
        Coefficient(0) = 0.0;
        Coefficient(1) = 2.0;
    }    
    else
    {
        doubleVector hres(n+1,0.0);
        doubleVector h_prev2(n+1,0.0);
        doubleVector h_prev1(n+1,0.0);

        h_prev2[0] = 1; // H(x|0) = 1
        h_prev1[1] = 2; // H(x|1) = 2x
        
        for(int i=2; i<=n; ++i)
        {
            for(int j=0;j<=n;j++)
                hres[j] = 0.0;
            // gen from recurrence relation
            for(int j=0;j<=n;j++)
            {
                hres[j] = -2 *(i-1) * h_prev2[j];
                if(j >0)
                    hres[j] += 2 * h_prev1[j-1];
            }
            
            if(i < n)
            {
                h_prev2 = h_prev1;
                h_prev1 = hres;
            }
        }
        for(int i=0;i<=n;i++)
            Coefficient(i) = hres[i];    
    }
    
}

// ****************************************************************************
//  Method:  avtHermitePolynomial::~avtHermitePolynomial
//
//  Purpose:
//     Hermite Polynomial destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtHermitePolynomial::~avtHermitePolynomial()
{;}




