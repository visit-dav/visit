// ************************************************************************* //
//                        avtAreaBetweenCurvesQuery.C                        //
// ************************************************************************* //

#include <avtAreaBetweenCurvesQuery.h>


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
    vector<float>  newCurve1Vals;
    vector<float>  newCurve2Vals;
    vector<float>  usedX;
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
// ****************************************************************************

std::string
avtAreaBetweenCurvesQuery::CreateMessage(double area)
{
    char msg[1024];
    sprintf(msg, "The area between the curves is %g.", area);
    std::string m = msg;
    return m;
}


