// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtAreaBetweenCurvesQuery.C                        //
// ************************************************************************* //

#include <avtAreaBetweenCurvesQuery.h>

#include <Utility.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::avtAreaBetweenCurvesQuery
//
//  Purpose:
//      Construct an avtAreaBetweenCurvesQuery object.
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtAreaBetweenCurvesQuery::avtAreaBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::~avtAreaBetweenCurvesQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtAreaBetweenCurvesQuery::~avtAreaBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::CompareCurves
//
//  Purpose:
//      Computes the area between two curves.
//
//  Programmer:   Hank Childs
//  Creation:     October 3, 2003
//
// ****************************************************************************

double
avtAreaBetweenCurvesQuery::CompareCurves(int n1, const float *x1, const float *y1,
                              int n2, const float *x2, const float *y2)
{
    std::vector<float>  newCurve1Vals;
    std::vector<float>  newCurve2Vals;
    std::vector<float>  usedX;
    PutOnSameXIntervals(n1, x1, y1, n2, x2, y2, usedX, newCurve1Vals,
                        newCurve2Vals);

    int total_n_pts = usedX.size();
    double sum = 0;
    for (int i = 0 ; i < total_n_pts-1 ; i++)
    {
        // each pair forms a trapezoid.  sum the areas of the trapezoids.
        // Note: this does not take into account the degenerate case where
        // the trapezoid doubles back on itself.
        double base1 = fabs(newCurve1Vals[i] - newCurve2Vals[i]);
        double base2 = fabs(newCurve1Vals[i+1] - newCurve2Vals[i+1]);
        double height = usedX[i+1] - usedX[i];
        double area = height * (base1 + base2) / 2.;
        sum += area;
    }

    return sum;
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::CreateMessage
//
//  Purpose:
//      Creates a message stating the area between curves.
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
avtAreaBetweenCurvesQuery::CreateMessage(double area)
{
    MapNode result_node;
    result_node["area_between_curves"] = area;
    queryAtts.SetXmlResult(result_node.ToXML());

    char msg[1024];
    std::string format = "The area between the curves is " 
                     + queryAtts.GetFloatFormat() +".";
    snprintf(msg,1024,format.c_str(), area);
    std::string m = msg;
    return m;
}


