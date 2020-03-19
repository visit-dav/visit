// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricAspectRatio.h                          //
// ************************************************************************* //

#ifndef AVT_VMETRIC_ASPECT_RATIO_H
#define AVT_VMETRIC_ASPECT_RATIO_H
#include <expression_exports.h>
#include <avtVerdictExpression.h> 

// ****************************************************************************
//  Class: avtVMetricAspectRatio
//
//  Purpose:
//      This metric measures aspect ratio.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricAspectRatio : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


