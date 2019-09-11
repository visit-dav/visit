// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricWarpage.h                              //
// ************************************************************************* //

#ifndef AVT_VMETRIC_WARPAGE_H
#define AVT_VMETRIC_WARPAGE_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricWarpage
//
//  Purpose:
//    This metric measures Warpage.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricWarpage : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


