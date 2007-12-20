/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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




