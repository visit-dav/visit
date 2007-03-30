// ************************************************************************* //
//                          avtVMetricAspectGamma.h                          //
// ************************************************************************* //

#ifndef AVT_VMETRIC_ASPECT_GAMMA_H
#define AVT_VMETRIC_ASPECT_GAMMA_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricAspectGamma
//
//  Purpose:
//      This metric measures Aspect Gamma.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricAspectGamma : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


