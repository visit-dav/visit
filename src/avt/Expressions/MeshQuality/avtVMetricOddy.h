// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVMetricOddy.h                                 //
// ************************************************************************* //

#ifndef AVT_VMETRIC_ODDY_H
#define AVT_VMETRIC_ODDY_H
#include <expression_exports.h>
#include <avtVerdictExpression.h>

// ****************************************************************************
//  Class: avtVMetricOddy
//
//  Purpose:
//    This metric measures oddy.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
// ****************************************************************************

class EXPRESSION_API avtVMetricOddy : public avtVerdictExpression
{
    public:
        double Metric(double coords[][3], int type);
};

#endif


