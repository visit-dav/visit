// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtExpectedValueQuery.C                         //
// ************************************************************************* //

#include <avtExpectedValueQuery.h>


// ****************************************************************************
//  Method: avtExpectedValueQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2006
//
// ****************************************************************************

avtExpectedValueQuery::avtExpectedValueQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtExpectedValueQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2006
//
// ****************************************************************************

avtExpectedValueQuery::~avtExpectedValueQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtExpectedValueQuery::CurveQuery
//
//  Purpose:
//      Computes the expected value: the integral of x*f(x) dx.
//
//  Programmer:   Hank Childs
//  Creation:     August 25, 2006
//
// ****************************************************************************

double
avtExpectedValueQuery::CurveQuery(int n1, const float *x1, const float *y1)
{
    //
    // Pretty straightforward.  We are going to calculate the integral as
    // the sum of a series of trapezoids.  We are not handling the case
    // where the trapezoid is degenerate (because the function crossed
    // the line y=0).  We will then multiply the trapezoid by the height
    // of the trapezoid.
    //
    double integral = 0;
    for (int i = 0 ; i < n1-1 ; i++)
    {
        double height = x1[i+1] - x1[i];
        double base1 = y1[i] - 0.;
        double base2 = y1[i+1] - 0.;
        integral += ((x1[i+1]+x1[i])/2.)*(base1+base2)*height / 2.;
    }

    return integral;
}


// ****************************************************************************
//  Method: avtExpectedValueQuery::CreateMessage
//
//  Purpose:
//      Creates a message for the integrate query.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2006
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Mon Feb 24 16:24:17 PST 2014
//    Add Xml results.
//
// ****************************************************************************

std::string
avtExpectedValueQuery::CreateMessage(double ev)
{
    MapNode result_node;
    result_node["expected_value"] = ev;
    SetXmlResult(result_node.ToXML());

    char msg[1024];
    std::string format = "The expected value is " + queryAtts.GetFloatFormat()
                         + ".";
    snprintf(msg,1024, format.c_str(), ev);
    std::string m = msg;
    return m;
}


