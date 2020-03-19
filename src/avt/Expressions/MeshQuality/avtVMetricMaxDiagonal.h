// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVMetricMaxDiagonal.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_MAX_DIAGONAL_H
#define AVT_VMETRIC_MAX_DIAGONAL_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricMaxDiagonal
//
//  Purpose:
//    This metric measures max diagonal length.
//
//  Programmer: Sean Ahern
//  Creation:   June 24, 2009
//
// ****************************************************************************

class EXPRESSION_API avtVMetricMaxDiagonal : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif
