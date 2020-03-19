// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtVMetricDiagonalRatio.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_DIAGONAL_RATIO_H
#define AVT_VMETRIC_DIAGONAL_RATIO_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricDiagonalRatio
//
//  Purpose:
//    This metric measures diagonal ratio.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricDiagonalRatio : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif
