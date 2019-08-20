// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricShapeAndSize.h                         //
// ************************************************************************* //

#ifndef AVT_VMETRIC_SHAPE_AND_SIZE_H
#define AVT_VMETRIC_SHAPE_AND_SIZE_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricShapeAndSize
//
//  Purpose:
//    This metric measures shape_and_size.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricShapeAndSize : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
    protected:
        virtual bool              RequiresSizeCalculation() { return true; }
};

#endif


