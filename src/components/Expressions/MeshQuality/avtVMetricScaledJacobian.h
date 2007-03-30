// ************************************************************************* //
//                          avtVMetricScaledJacobian.h                       //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SCALED_JACOBIAN_H
#define AVT_VMETRIC_SCALED_JACOBIAN_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricScaledJacobian
//
//  Purpose:
//    This metric measures scaled jacobian.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricScaledJacobian : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


