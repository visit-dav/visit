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

// ************************************************************************* //
//                              avtKurtosisQuery.C                           //
// ************************************************************************* //

#include <avtKurtosisQuery.h>

#include <stdio.h>
#include <math.h>


// ****************************************************************************
//  Method: avtKurtosisQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
// ****************************************************************************

avtKurtosisQuery::avtKurtosisQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtKurtosisQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
// ****************************************************************************

avtKurtosisQuery::~avtKurtosisQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtKurtosisQuery::CurveQuery
//
//  Purpose:
//      Computes the kurtosis.
//
//  Programmer:   Hank Childs
//  Creation:     August 5, 2005
//
// ****************************************************************************

double
avtKurtosisQuery::CurveQuery(int n1, const float *x1, const float *y1)
{
    // y1[i] should be interpreted as the probability of x1[i] occurring.

    int  i;

    // The mean is the raw moment, "mu prime, subscript=1" on the website
    // referenced in the header file.  It is the integral of x*P(x)dx.
    double mean = 0.;
    for (i = 0 ; i < n1-1 ; i++)
    {
        // We are now dealing with the interval from X[i] to X[i+1].
        // We want to calculate the integral of P(X)*X over this interval.
        // P(X) is a line segment over the interval.  We can calculate the
        // form of this line as P(X) = MX+B for some M and B.  Then the
        // integral is over X*(MX+B) = MX^2 + BX.  So the integral is
        // MX^3/3 + BX^2/2.
        //
        // Then the gameplan is:
        // 1) Calculate M and B.
        // 2) Calculate contribution from this interval.
        // 3) Add to total mean, since the integral is just the sum of
        //    the contributions from each sub-interval.
     
        double M = 0.;
        if (x1[i] != x1[i+1])
            M = (y1[i+1] - y1[i]) / (x1[i+1] - x1[i]);
        double B = y1[i] - M*x1[i];

        double high_end = M*x1[i+1]*x1[i+1]*x1[i+1] / 3. 
                        + B*x1[i+1]*x1[i+1] / 2.;
        double low_end  = M*x1[i]*x1[i]*x1[i] / 3. 
                        + B*x1[i]*x1[i] / 2.;
        double contribution = high_end - low_end;

        mean += contribution;
    }

    double moment_4 = 0.;
    double moment_2 = 0.;
    for (i = 0 ; i < n1-1 ; i++)
    {
        // We have a similar game plan to above, but now we are calculating
        // higher moments (the mean is the first moment).  However, we have
        // to integrate hairier functions to get these moments.
        // 4th central moment: (X-mean)^4*(MX+B)
        // 2nd central moment: (X-mean)^2*(MX+B)
        // (central moment --> normalize by mean).
        //
        // (X-mean) ^ 4 means that we are going to have a nasty polynomial.
        // So let's do a trick.  Integrate over Y, where Y=(X-mean).
        
        // Still start off by calculating the line parameters.
        double M = 0.;
        if (x1[i] != x1[i+1])
            M = (y1[i+1] - y1[i]) / (x1[i+1] - x1[i]);
        double B = y1[i] - M*x1[i];

        double si = x1[i]-mean;   // start of interval
        double ei = x1[i+1]-mean; // end of interval

        // MX+B ==> M*(Y+mean)+B = M*Y + M*mean+B
        double B_adj = M*mean+B;

        // 4th moment integrand: Y^4*(M*Y+B_adj) = M*Y^5 + B_adj*Y^4.
        // 4th moment integral:  M*Y^6/6 + B_adj*Y^5/5 (over new interval).
        double moment_4_high_end = M*pow(ei, 6.0)/6.0 + B_adj*pow(ei, 5.0)/5.0;
        double moment_4_low_end  = M*pow(si, 6.0)/6.0 + B_adj*pow(si, 5.0)/5.0;
        moment_4 += (moment_4_high_end - moment_4_low_end);

        // 2nd moment integrand: Y^2*(M*Y+B_adj) = M*Y^3 + B_adj*Y^2.
        // 2nd moment integral:  M*Y^4/4 + B_adj*Y^3/3 (over new interval).
        double moment_2_high_end = M*pow(ei, 4.0)/4.0 + B_adj*pow(ei, 3.0)/3.0;
        double moment_2_low_end  = M*pow(si, 4.0)/4.0 + B_adj*pow(si, 3.0)/3.0;
        moment_2 += (moment_2_high_end - moment_2_low_end);
    }

    // Kurtosis is the 4th moment divided by the square of the second moment.
    return moment_4 / (moment_2*moment_2);
}


// ****************************************************************************
//  Method: avtKurtosisQuery::CreateMessage
//
//  Purpose:
//      Creates a message for the integrate query.
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
// ****************************************************************************

std::string
avtKurtosisQuery::CreateMessage(double kurtosis)
{
    char msg[1024];
    sprintf(msg, "The kurtosis of the distribution is %g.", kurtosis);
    std::string m = msg;
    return m;
}


