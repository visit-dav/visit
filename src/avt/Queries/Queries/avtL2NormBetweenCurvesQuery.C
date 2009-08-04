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
//                        avtL2NormBetweenCurvesQuery.C                      //
// ************************************************************************* //

#include <avtL2NormBetweenCurvesQuery.h>

#include <snprintf.h>
#include <Utility.h>


// ****************************************************************************
//  Method: avtL2NormBetweenCurvesQuery::avtL2NormBetweenCurvesQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtL2NormBetweenCurvesQuery::avtL2NormBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtL2NormBetweenCurvesQuery::~avtL2NormBetweenCurvesQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtL2NormBetweenCurvesQuery::~avtL2NormBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtL2NormBetweenCurvesQuery::CompareCurves
//
//  Purpose:
//      Computes the L2Norm number of the input curves.
//
//  Notes:
//      For two curves, c_a and c_b, let f(x) = |c_a(x) - c_b(x)|.
//      Then the L2-Norm is defined to be the integral of f(x)^2 over x.
//      
//      The curves are already broken up into line segments.  But those line
//      segments have differing endpoints across the curves.  (Meaning curve
//      1 may have a line segment from 0-0.435 in X, but curve 2's has a line
//      segment that goes from 0-0.546.  There are many more permutations of
//      this -- curve 2 may devote 3 segments to an interval in X that curve
//      1 covers in 1 line segment.)
//     
//      The easiest way to handle this problem is to create new curves that
//      have the same number of line segments and have the property that line
//      segment i for the new curves covers exactly the same interval in X.
//     
//      Once we have done that, we can start to consider what the contribution
//      of each line segment is to the L2-Norm.  Over line segment i, the
//      function f(x) is a line.  The line has the form f(x) = mx+b.  We also
//      know two points on the line -- f(x1) = |c1(x1) - c2(x1)| and
//      f(x2) = |c1(x2) - c2(x2)|.  Using these points we can calculate m and b.
//     
//      Once we know m and b, we can calculate what f(x)^2 is.  It is a 
//      parabola.  Finally, once we know f(x)^2, we can calculate the integral 
//      from x1 to x2.  
//      It is:
//      m*m*x1*x1*x1/3 + mb*x1*x1 + b*b*x1 - m*m*x2*x2*x2/3 + mb*x2*x2 + b*b*x2 
//     
//      So our gameplan is to break up the curves into line segments that run
//      over the same X-intervals, then calculate the function f(x), which is
//      a line, along that segment, and then determine that line segment's 
//      total contribution to the integral.  Once we have considered all of
//      the line segments, we will have calculated the whole integral.
//     
//  Programmer:   Hank Childs
//  Creation:     October 3, 2003
//
// ****************************************************************************

double
avtL2NormBetweenCurvesQuery::CompareCurves(int n1, const float *x1,
                     const float *y1, int n2, const float *x2, const float *y2)
{
    vector<float>  newCurve1Vals;
    vector<float>  newCurve2Vals;
    vector<float>  usedX;
    PutOnSameXIntervals(n1, x1, y1, n2, x2, y2, usedX, newCurve1Vals,
                        newCurve2Vals);

    int total_n_pts = usedX.size();
    double integral = 0;
    for (int i = 0 ; i < total_n_pts-1 ; i++)
    {
        // pt1 and pt2 will determine f(x) for the range usedX[i]-[i+1]
        float pt1[2], pt2[2];
        pt1[0] = usedX[i];
        pt1[1] = newCurve1Vals[i] - newCurve2Vals[i];
        pt2[0] = usedX[i+1];
        pt2[1] = newCurve1Vals[i+1] - newCurve2Vals[i+1];

        // Calculate the line in f(x) = mx+b notation. 
        // Note that pt1[0] != pt2[0] because we threw out duplicated points.
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
//  Method: avtL2NormBetweenCurvesQuery::CreateMessage
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
avtL2NormBetweenCurvesQuery::CreateMessage(double l2norm)
{
    char msg[1024];
    string format = "The L2Norm between the two curves is " 
                    + queryAtts.GetFloatFormat() +".";
    SNPRINTF(msg,1024,format.c_str(), l2norm);
    std::string m = msg;
    return m;
}


