// ************************************************************************* //
//                          avtVMetricLargestAngle.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_LargestAngle_H
#define AVT_VMETRIC_LargestAngle_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricLargestAngle
//
//  Purpose:
//    This metric measures LargestAngle.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricLargestAngle : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


