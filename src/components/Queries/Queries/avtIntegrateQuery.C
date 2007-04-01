// ************************************************************************* //
//                             avtIntegrateQuery.C                           //
// ************************************************************************* //

#include <avtIntegrateQuery.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtIntegrateQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIntegrateQuery::avtIntegrateQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtIntegrateQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtIntegrateQuery::~avtIntegrateQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtIntegrateQuery::CurveQuery
//
//  Purpose:
//      Computes the area under the curve -- the integral.
//
//  Programmer:   Hank Childs
//  Creation:     October 3, 2003
//
// ****************************************************************************

double
avtIntegrateQuery::CurveQuery(int n1, const float *x1, const float *y1)
{
    //
    // Pretty straightforward.  We are going to calculate the integral as
    // the sum of a series of trapezoids.  We are not handling the case
    // where the trapezoid is degenerate (because the function crossed
    // the line y=0).
    //
    double integral = 0;
    for (int i = 0 ; i < n1-1 ; i++)
    {
        double height = x1[i+1] - x1[i];
        double base1 = y1[i] - 0.;
        double base2 = y1[i+1] - 0.;
        integral += (base1+base2)*height / 2.;
    }

    return sqrt(integral);
}


// ****************************************************************************
//  Method: avtIntegrateQuery::CreateMessage
//
//  Purpose:
//      Creates a message for the integrate query.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

std::string
avtIntegrateQuery::CreateMessage(double integral)
{
    char msg[1024];
    sprintf(msg, "The area under the curve is %g.", integral);
    std::string m = msg;
    return m;
}


