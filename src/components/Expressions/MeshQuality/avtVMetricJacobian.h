// ************************************************************************* //
//                          avtVMetricJacobian.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_JACOBIAN_H
#define AVT_VMETRIC_JACOBIAN_H
#include <expression_exports.h>
#include <avtVerdictFilter.h>

// ****************************************************************************
//  Class: avtVMetricJacobian
//
//  Purpose:
//    This metric measures jacobian.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricJacobian : public avtVerdictFilter
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


