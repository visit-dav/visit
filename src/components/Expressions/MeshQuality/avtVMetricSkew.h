// ************************************************************************* //
//                          avtVMetricSkew.h                                 //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SKEW_H
#define AVT_VMETRIC_SKEW_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricSkew
//
//  Purpose:
//    This metric measures an element's skew.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricSkew : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


