// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricMaximumAngle.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_MaximumAngle_H
#define AVT_VMETRIC_MaximumAngle_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricMaximumAngle
//
//  Purpose:
//    This metric measures MaximumAngle.
//
//  Programmer: Eric Brugger
//  Creation:   July 31, 2008
//
// ****************************************************************************

class EXPRESSION_API avtVMetricMaximumAngle : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


