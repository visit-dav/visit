// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricShape.h                                //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHAPE_H
#define AVT_VMETRIC_SHAPE_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricShape
//
//  Purpose:
//    This metric measures shape.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricShape : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


