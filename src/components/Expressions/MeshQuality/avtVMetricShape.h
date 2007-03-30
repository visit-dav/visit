// ************************************************************************* //
//                          avtVMetricShape.h                                //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHAPE_H
#define AVT_VMETRIC_SHAPE_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricShape
//
//  Purpose:
//    This metric measures shape.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricShape : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


