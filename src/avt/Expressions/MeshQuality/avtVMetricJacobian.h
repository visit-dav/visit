// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricJacobian.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_JACOBIAN_H
#define AVT_VMETRIC_JACOBIAN_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

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

class EXPRESSION_API avtVMetricJacobian : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


