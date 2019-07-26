// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtIntegrateQuery.C                           //
// ************************************************************************* //

#include <avtIntegrateQuery.h>

#include <snprintf.h>

#include <string>

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
//  Modifications:
//
//    Hank Childs, Sat Feb 14 09:59:51 PST 2004
//    Fix cut-n-paste bug.
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

    return integral;
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
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Thu Feb 13 15:04:58 PST 2014
//    Add Xml results.
//
// ****************************************************************************

std::string
avtIntegrateQuery::CreateMessage(double integral)
{
    MapNode result_node;
    result_node["area_under_curve"] = integral;
    SetXmlResult(result_node.ToXML());

    char msg[1024];
    std::string format =  "The area under the curve is " 
                         + queryAtts.GetFloatFormat() + ".";
    SNPRINTF(msg,1024, format.c_str(),integral);
    std::string m = msg;
    return m;
}


