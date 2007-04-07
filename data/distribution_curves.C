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
//                            distribution_curves.C                          //
// ************************************************************************* //

#include <visitstream.h>
#include <stdio.h>
#include <math.h>

// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Creates Ultra files for well known distributions (useful for testing
//      queries like kurtosis and skewness).
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
//  Modification:
//    Kathleen Bonnell, Fri Oct 28 14:27:12 PDT 2005
//    Add TIME to end of file.
//
// ****************************************************************************

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main()
{
    int  i, j;
    int npts = 50000;

    ofstream ofile("distribution.ultra");

    ofile << "# Laplace Distribution" << endl;
    double mean = 0.;
    double b = 1.;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*((i-(npts/2.)) / (npts/2.));
        double LP = 1/(2.*b) * pow(M_E, -fabs(X-mean)/b);
        ofile << X << " " << LP << endl;
    }

    ofile << "# Log Normal Distribution" << endl;
    double S = 1.;
    double M = 0.;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*((i+1.) / npts);
        double denom = 1./(S*sqrt(2*M_PI)*X);
        double exp = -log(X-M)*log(X-M) / (2*S*S);
        double log_normal = denom * pow(M_E, exp);
        ofile << X << "\t" << log_normal << endl;
    }

    ofile << "# Exponential Distribution" << endl;
    double lambda = 1;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*(((float)i) / npts);
        ofile << X << "\t" << pow(M_E, -lambda*X) << endl;
    }
    ofile << "# TIME 56.789" << endl;
}


