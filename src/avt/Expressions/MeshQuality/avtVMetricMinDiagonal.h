// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVMetricMinDiagonal.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_MIN_DIAGONAL_H
#define AVT_VMETRIC_MIN_DIAGONAL_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricMinDiagonal
//
//  Purpose:
//    This metric measures min diagonal length.
//
//  Programmer: Sean Ahern
//  Creation:   June 24, 2009
//
// ****************************************************************************

class EXPRESSION_API avtVMetricMinDiagonal : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif
