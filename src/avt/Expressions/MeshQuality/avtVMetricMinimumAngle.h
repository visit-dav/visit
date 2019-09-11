// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricMinimumAngle.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_MinimumAngle_H
#define AVT_VMETRIC_MinimumAngle_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricMinimumAngle
//
//  Purpose:
//   This metric measures MinimumAngle.
//
//  Programmer: Eric Brugger
//  Creation:   July 31, 2008
//
// ****************************************************************************

class EXPRESSION_API avtVMetricMinimumAngle : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


