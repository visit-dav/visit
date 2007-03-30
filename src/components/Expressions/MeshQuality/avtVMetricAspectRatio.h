// ************************************************************************* //
//                          avtVMetricAspectRatio.h                          //
// ************************************************************************* //

#ifndef AVT_VMETRIC_ASPECT_RATIO_H
#define AVT_VMETRIC_ASPECT_RATIO_H
#include <expression_exports.h>
#include <avtVerdictFilter.h> 

// ****************************************************************************
//  Class: avtVMetricAspectRatio
//
//  Purpose:
//      This metric measures aspect ratio.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricAspectRatio : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


