// ************************************************************************* //
//                              avtL2NormQuery.C                             //
// ************************************************************************* //

#include <avtL2NormQuery.h>

#include <stdio.h>


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
//  Method: avtL2NormQuery::CreateMessage
//
//  Purpose:
//      Creates a message for the L2Norm query.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

std::string
avtL2NormQuery::CreateMessage(double l2norm)
{
    char msg[1024];
    sprintf(msg, "The L2Norm is %g.", l2norm);
    std::string m = msg;
    return m;
}


