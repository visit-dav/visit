// ************************************************************************* //
//                          avtVMetricDimension.h                            //
// ************************************************************************* //

#ifndef AVT_VMETRIC_DIMENSION_H
#define AVT_VMETRIC_DIMENSION_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricDimension
//
//  Purpose:
//    This metric measures dimension.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricDimension : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


