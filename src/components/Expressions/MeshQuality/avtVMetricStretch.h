// ************************************************************************* //
//                          avtVMetricStretch.h                              //
// ************************************************************************* //

#ifndef AVT_VMETRIC_STRETCH_H
#define AVT_VMETRIC_STRETCH_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricStretch
//
//  Purpose:
//    This metric measures aspect ratio.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricStretch : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


