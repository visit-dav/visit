// ************************************************************************* //
//                          avtVMetricSmallestAngle.h                        //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SmallestAngle_H
#define AVT_VMETRIC_SmallestAngle_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricSmallestAngle
//
//  Purpose:
//   This metric measures SmallestAngle.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricSmallestAngle : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


