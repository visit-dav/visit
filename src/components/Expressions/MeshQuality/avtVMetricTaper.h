// ************************************************************************* //
//                          avtVMetricTaper.h                                //
// ************************************************************************* //

#ifndef AVT_VMETRIC_TAPER_H
#define AVT_VMETRIC_TAPER_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricTaper
//
//  Purpose:
//    This metric measures taper.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricTaper : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


