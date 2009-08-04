/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


