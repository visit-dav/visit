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

// ************************************************************************* //
//                              avtL2NormQuery.C                             //
// ************************************************************************* //

#include <avtL2NormQuery.h>

#include <snprintf.h>


// ****************************************************************************
//  Method: avtL2NormQuery::avtL2NormQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtL2NormQuery::avtL2NormQuery()
{
}


// ****************************************************************************
//  Method: avtL2NormQuery::~avtL2NormQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtL2NormQuery::~avtL2NormQuery()
{
}


// ****************************************************************************
//  Method: avtL2NormQuery::CurveQuery
//
//  Purpose:
//      Computes the L2Norm number of the input curve.
//
//  Notes:
//      The L2Norm can be calculated as the sum of contributions from the 
//      line segments that make up the curve.  Over line segment i, the
//      function f(x) is a line.  The line has the form f(x) = mx+b. 
//      We can easily calculate m and b.
//     
//      Once we know m and b, we can calculate what f(x)^2 is.  It is a 
//      parabola.  Finally, once we know f(x)^2, we can calculate the integral 
//      from x1 to x2.  
//      It is:
//      m*m*x1*x1*x1/3 + mb*x1*x1 + b*b*x1 - m*m*x2*x2*x2/3 + mb*x2*x2 + b*b*x2 
//     
//  Programmer:   Hank Childs
//  Creation:     October 3, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Mar 15 15:43:31 PST 2005
//    The assumption that x-coords won't be duplicated came from the original
//    implementation of this query -- the L2-norm between curves query.  That
//    assumption is not true for this query, so add explicit checking.
//
// ****************************************************************************

double
avtL2NormQuery::CurveQuery(int n1, const float *x1, const float *y1)
{
    double integral = 0;
    for (int i = 0 ; i < n1-1 ; i++)
    {
        // pt1 and pt2 will determine f(x) for the range usedX[i]-[i+1]
        float pt1[2], pt2[2];
        pt1[0] = x1[i];
        pt1[1] = y1[i];
        pt2[0] = x1[i+1];
        pt2[1] = y1[i+1];

        // Degeneracies like this can occur from lineouts due to numerical
        // sensititives.  They also can be in a file from a database (like
        // a pre-existing degeneracy in an Ultra file).
        if (pt1[0] == pt2[0])
            continue;

        // Calculate the line in f(x) = mx+b notation. 
        double rise = pt2[1] - pt1[1];
        double run  = pt2[0] - pt1[0];
        double m = rise / run;
        double b = pt1[1] - m*pt1[0];
        integral += 
                m*m*pt2[0]*pt2[0]*pt2[0]/3. + m*b*pt2[0]*pt2[0] + b*b*pt2[0]
              - m*m*pt1[0]*pt1[0]*pt1[0]/3. - m*b*pt1[0]*pt1[0] - b*b*pt1[0];
    }

    return sqrt(integral);
}


// ****************************************************************************
//  Method: avtL2NormQuery::CreateMessage
//
//  Purpose:
//      Creates a message for the L2Norm query.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

std::string
avtL2NormQuery::CreateMessage(double l2norm)
{
    char msg[1024];
    string format = "The L2Norm is " + queryAtts.GetFloatFormat() + ".";
    SNPRINTF(msg,1024,format.c_str(), l2norm);
    std::string m = msg;
    return m;
}


